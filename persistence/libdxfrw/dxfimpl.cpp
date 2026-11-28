#include "dxfimpl.h"
#include "../generic/helpers.h"
#include "mtextcodec.h"

#include "../patternLoader/patternProvider.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <cad/primitive/circle.h>
#include <cad/primitive/hatch.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/dimension.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/dimordinate.h>
#include <cad/primitive/point.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/operations/entitybuilder.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/operations/linepatternops.h>
#include <cad/meta/color.h>
#include <cad/meta/icolor.h>
#include <cad/base/metainfo.h>
#include <cad/meta/metacolor.h>
#include <cad/meta/metalinewidth.h>
#include <cad/meta/dxflinepattern.h>
#include <cad/tools/string_helper.h>
#include <cad/meta/block.h>
#include <cad/primitive/insert.h>
#include <cad/operations/blockops.h>
#include <cad/meta/customentitystorage.h>
#include <cad/meta/imagedisplaysource.h>
#include <cad/logger/logger.h>
#include <cad/tools/maphelper.h>

using namespace lc::persistence;

namespace {
struct ImageDefinitionSource {
    std::string path;
    int version{0}, loaded{0}, resolution{0};
    double u{0}, v{0}, up{0}, vp{0};
    bool pathCaptured{false}, unmodeledMetadata{false};
    std::list<std::list<DRW_Variant>> appData;
    std::vector<DRW_Variant> xdata;
    std::vector<std::weak_ptr<const lc::meta::Layer>> xdataLayers;
};

bool sourceHandle(const DRW_Variant& value, std::uint32_t& handle);

#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
bool simpleImageAscii(const std::string& value, std::size_t limit) {
    return value.size() <= limit && std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return c >= 0x20 && c <= 0x7e && c != '\\';
    });
}

bool captureSimpleImageDefinitionAppData(const std::list<std::list<DRW_Variant>>& source,
                                         std::list<std::list<DRW_Variant>>& target,
                                         std::size_t& retainedBytes) {
    constexpr std::size_t kMaxBytes = 4u * 1024u * 1024u;
    if (source.size() > 64 || retainedBytes > kMaxBytes) return false;
    std::size_t items = 0;
    bool allRetained = true;
    for (const auto& group : source) {
        std::size_t cost = 0;
        const auto charge = [&](std::size_t bytes) {
            if (bytes > kMaxBytes - retainedBytes - cost) return false;
            cost += bytes;
            return true;
        };
        const bool safe = [&]() {
            if (group.size() < 2 || group.size() > 128 || items > 128 - group.size() ||
                !charge(sizeof(group) + 2 * sizeof(void*))) return false;
            const auto current = group.begin();
            if (current->code() != 102 || current->type() != DRW_Variant::STRING || !current->content.s ||
                current->content.s->empty() || !simpleImageAscii(*current->content.s, 255)) return false;
            std::string marker = *current->content.s;
            if (marker.front() == '{') marker.erase(marker.begin());
            if (marker.empty() || marker.size() > 254 || marker.find_first_of("{}") != std::string::npos) return false;
            std::transform(marker.begin(), marker.end(), marker.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            if (marker == "ACAD_REACTORS" || marker == "ACAD_XDICTIONARY") return false;
            for (auto it = group.begin(); it != group.end(); ++it) {
                const bool first = it == group.begin();
                const bool last = std::next(it) == group.end();
                if (it->type() != DRW_Variant::STRING || !it->content.s || !simpleImageAscii(*it->content.s, 255) ||
                    (first ? it->code() != 102 : last ? (it->code() != 102 || *it->content.s != "}") : it->code() != 1) ||
                    !charge(sizeof(DRW_Variant) + 2 * sizeof(void*)) || !charge(it->content.s->size())) return false;
            }
            return true;
        }();
        if (!safe) { allRetained = false; continue; }
        target.push_back(group);
        items += group.size();
        retainedBytes += cost;
    }
    return allRetained;
}

bool captureSimpleImageDefinitionXdata(const std::vector<DRW_Variant*>& source,
                                       std::vector<DRW_Variant>& target, std::size_t& retainedBytes) {
    constexpr std::size_t kMaxBytes = 4u * 1024u * 1024u;
    if (source.size() > 128 || retainedBytes > kMaxBytes) return false;
    std::set<std::string> seen;
    bool allRetained = true;
    for (std::size_t begin = 0; begin < source.size();) {
        if (!source[begin] || source[begin]->code() != 1001) return false;
        std::size_t end = begin + 1;
        while (end < source.size() && source[end] && source[end]->code() != 1001) ++end;
        if (end < source.size() && !source[end]) return false;
        std::size_t cost = 0;
        const auto charge = [&](std::size_t bytes) {
            if (bytes > kMaxBytes - retainedBytes - cost) return false;
            cost += bytes;
            return true;
        };
        std::string key;
        const bool safe = [&]() {
            const auto* app = source[begin];
            if (app->type() != DRW_Variant::STRING || !app->content.s || app->content.s->empty() ||
                !simpleImageAscii(*app->content.s, 31) ||
                !std::all_of(app->content.s->begin(), app->content.s->end(), [](unsigned char c) {
                    return std::isalnum(c) || c == '_' || c == '$' || c == '-';
                })) return false;
            key = *app->content.s;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            if (seen.count(key)) return false;
            unsigned depth = 0;
            for (std::size_t i = begin; i < end; ++i) {
                const auto* value = source[i];
                if (!value || !charge(sizeof(DRW_Variant) + sizeof(void*) + sizeof(std::weak_ptr<const lc::meta::Layer>))) return false;
                switch (value->code()) {
                case 1001:
                    if (i != begin || !charge(value->content.s->size())) return false;
                    break;
                case 1000:
                    if (value->type() != DRW_Variant::STRING || !value->content.s ||
                        !simpleImageAscii(*value->content.s, 255) || !charge(value->content.s->size())) return false;
                    break;
                case 1003:
                    if (value->type() != DRW_Variant::STRING || !value->content.s || value->content.s->empty() ||
                        value->activeDwgLayerReference() || !simpleImageAscii(*value->content.s, 255) ||
                        !charge(value->content.s->size())) return false;
                    break;
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
                case 1005: {
                    std::uint32_t handle = 0;
                    if (!sourceHandle(*value, handle) || handle == 0 || handle == DRW::NoHandle ||
                        !charge(value->content.s->size())) return false;
                    break;
                }
#endif
                case 1002:
                    if (value->type() != DRW_Variant::STRING || !value->content.s || !charge(value->content.s->size())) return false;
                    if (*value->content.s == "{") { if (++depth > 16) return false; }
                    else if (*value->content.s == "}") { if (!depth) return false; --depth; }
                    else return false;
                    break;
                case 1040: case 1041: case 1042:
                    if (value->type() != DRW_Variant::DOUBLE || !std::isfinite(value->content.d)) return false;
                    break;
                case 1070:
                    if (value->type() != DRW_Variant::INTEGER || value->i_val() < std::numeric_limits<std::int16_t>::min() ||
                        value->i_val() > std::numeric_limits<std::int16_t>::max()) return false;
                    break;
                case 1071:
                    if (value->type() != DRW_Variant::INTEGER) return false;
                    break;
                default:
                    return false; // Other reference and opaque values need separate graph proof.
                }
            }
            return depth == 0;
        }();
        if (safe) {
            for (std::size_t i = begin; i < end; ++i) target.push_back(*source[i]);
            retainedBytes += cost;
            seen.insert(key);
        } else allRetained = false;
        begin = end;
    }
    return allRetained;
}
#endif

bool validImageDefinitionSource(const ImageDefinitionSource& source, const std::string& path) {
    return source.pathCaptured && !source.path.empty() && source.path == path &&
        source.version >= 0 && source.version <= DRW_ImageDef::kMaxClassVersion &&
        std::isfinite(source.u) && std::isfinite(source.v) &&
        std::isfinite(source.up) && std::isfinite(source.vp) && source.u >= 0 && source.v >= 0 &&
        source.loaded >= 0 && source.loaded <= 1 && source.resolution >= 0 && source.resolution <= 0xff;
}

// Pixel-space clipping stays valid through the Image transforms, which change
// its world-space axes rather than its pixel dimensions. Keep a bounded copy
// with the entity metadata so copies retain the same source settings.
class ImageDisplaySource final : public lc::meta::EntityMetaType {
public:
    static const std::string key() { return lc::meta::detail::imageDisplaySourceKey(); }
    const std::string id() const override { return key(); }
    const std::string metaTypeID() const override { return key(); }
    int classVersion{0};
    int displayProps{0};
    int clip{0};
    int boundaryType{0};
    bool clipMode{false};
    bool boundaryDropped{false};
    std::uint32_t definitionHandle{0};
    std::shared_ptr<const void> sourceIdentity;
    std::shared_ptr<const ImageDefinitionSource> definition;
    double width{0};
    double height{0};
    std::vector<DRW_Coord> boundary;
};

bool regeneratesImageObject(const DRW_RawDxfObject& object, bool hasImages) {
    return hasImages && (object.name == "IMAGEDEF" || object.name == "IMAGEDEF_REACTOR");
}

bool skipsSourceImageObject(const DRW_RawDxfObject& object, bool hasImages,
                            const PreservedRecords* source) {
    return regeneratesImageObject(object, hasImages) ||
        (!hasImages && source && object.name == "IMAGEDEF_REACTOR" &&
         source->sourceImageReactorHandles.count(object.handle));
}

bool sourceHandle(const DRW_Variant& value, std::uint32_t& handle) {
    if (value.type() != DRW_Variant::STRING || !value.content.s) return false;
    const auto& hex = *value.content.s;
    if (hex.empty() || hex.size() > 8 ||
        !std::all_of(hex.begin(), hex.end(), [](unsigned char c) { return std::isxdigit(c); })) return false;
    handle = static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
    return true;
}

// Return the positions and source targets of canonical name/soft-owner pairs.
bool ordinaryDictionaryEntries(const DRW_RawDxfObject& object,
                               std::vector<std::pair<std::size_t, std::uint32_t>>& entries) {
    const auto& groups = object.groups;
    std::uint32_t handle = 0, owner = 0;
    if (object.name != "DICTIONARY" || groups.size() < 3 ||
        object.rawValues.size() != groups.size() || groups[0].code() != 5 ||
        groups[1].code() != 330 || groups[2].code() != 100 ||
        !sourceHandle(groups[0], handle) || handle != object.handle ||
        !sourceHandle(groups[1], owner) || owner != object.parentHandle ||
        groups[2].type() != DRW_Variant::STRING || !groups[2].content.s ||
        *groups[2].content.s != "AcDbDictionary") return false;
    std::size_t pos = 3;
    if (pos < groups.size() && groups[pos].code() == 280) {
        if (groups[pos].type() != DRW_Variant::INTEGER) return false;
        ++pos;
    }
    if (pos < groups.size() && groups[pos].code() == 281) {
        if (groups[pos].type() != DRW_Variant::INTEGER) return false;
        ++pos;
    }
    while (pos < groups.size()) {
        if (pos + 1 >= groups.size() || groups[pos].code() != 3 ||
            groups[pos].type() != DRW_Variant::STRING || !groups[pos].content.s ||
            groups[pos].content.s->empty() || groups[pos + 1].code() != 350 ||
            !sourceHandle(groups[pos + 1], handle)) return false;
        entries.emplace_back(pos, handle);
        pos += 2;
    }
    return true;
}

#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
// Admit only a bounded, metadata-free source dictionary whose physical pairs
// agree with its typed view. Other dictionaries retain the existing fallback.
bool simpleImageDictionaryTyped(const DRW_Dictionary& dict) {
    if (dict.tType != DRW::DICTIONARY || dict.handle == 0 || dict.handle == DRW::NoHandle ||
        dict.m_entries.empty() || dict.m_entries.size() > 128 || !dict.hasCompleteDxfEntries() ||
        !dict.hasCompleteDwgEntries() || dict.hardOwner < 0 || dict.hardOwner > 1 ||
        dict.cloning < 0 || dict.cloning > 32767 || dict.countCap || dict.hasDataStorageBinaryData() ||
        !dict.appData.empty() || !dict.extData.empty() || !dict.reactorHandles.empty() ||
        !dict.pendingAppIdResolutions.empty() || !dict.pendingLayerRefResolutions.empty() ||
        dict.xDictHandle != 0) return false;
    std::set<std::string> names;
    std::set<std::uint32_t> handles;
    for (const auto& entry : dict.m_entries) {
        if (entry.m_name.empty() || !simpleImageAscii(entry.m_name, 255) || entry.m_handle == 0 ||
            entry.m_handle == DRW::NoHandle ||
            entry.m_dxfHandleCode != DRW_Dictionary::Entry::DxfHandleCode::SoftOwner) return false;
        auto name = entry.m_name;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        if (!names.insert(name).second || !handles.insert(entry.m_handle).second) return false;
    }
    return true;
}

bool simpleImageDictionaryMatches(const DRW_Dictionary& dict, const DRW_RawDxfObject& raw) {
    std::vector<std::pair<std::size_t, std::uint32_t>> entries;
    if (!simpleImageDictionaryTyped(dict) || !raw.hasRawValues || raw.m_version < DRW::AC1015 ||
        raw.groups.size() > 3 + 2 + 2 * 128 ||
        raw.handle != dict.handle || raw.parentHandle != dict.parentHandle ||
        !ordinaryDictionaryEntries(raw, entries) || entries.size() != dict.m_entries.size()) return false;
    for (const auto& group : raw.groups) {
        if ((group.code() == 280 && group.content.i != dict.hardOwner) ||
            (group.code() == 281 && group.content.i != dict.cloning)) return false;
    }
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const auto pos = entries[i].first;
        if (entries[i].second != dict.m_entries[i].m_handle ||
            *raw.groups[pos].content.s != dict.m_entries[i].m_name ||
            raw.rawValues[pos] != dict.m_entries[i].m_name) return false;
    }
    return true;
}
#endif

bool simpleTextXRecord(const DRW_RawDxfObject& record, std::uint32_t expectedOwner) {
    const auto& fields = record.groups;
    std::uint32_t handle = 0, owner = 0;
    return record.name == "XRECORD" && record.parentHandle == expectedOwner &&
        record.rawValues.size() == fields.size() && fields.size() >= 4 &&
        fields[0].code() == 5 && sourceHandle(fields[0], handle) && handle == record.handle &&
        fields[1].code() == 330 && sourceHandle(fields[1], owner) && owner == expectedOwner &&
        fields[2].code() == 100 && fields[2].type() == DRW_Variant::STRING &&
        fields[2].content.s && *fields[2].content.s == "AcDbXrecord" &&
        fields[3].code() == 280 && fields[3].type() == DRW_Variant::INTEGER &&
        std::all_of(fields.begin() + 4, fields.end(), [](const DRW_Variant& field) {
            return field.code() == 1 && field.type() == DRW_Variant::STRING;
        });
}

// Unknown fields or application groups leave the complete object untouched.
std::size_t dropRegeneratedImageEntries(DRW_RawDxfObject& object,
                                        const std::unordered_set<std::uint32_t>& skipped) {
    std::vector<std::pair<std::size_t, std::uint32_t>> entries;
    if (!ordinaryDictionaryEntries(object, entries)) return 0;
    const auto& groups = object.groups;
    std::vector<bool> remove(groups.size(), false);
    std::size_t count = 0;
    for (const auto& entry : entries) {
        if (skipped.count(entry.second)) {
            remove[entry.first] = remove[entry.first + 1] = true;
            ++count;
        }
    }
    if (count == 0) return 0;
    std::vector<DRW_Variant> keptGroups;
    std::vector<UTF8STRING> keptRaw;
    keptGroups.reserve(groups.size() - 2 * count);
    keptRaw.reserve(groups.size() - 2 * count);
    for (std::size_t i = 0; i < groups.size(); ++i) {
        if (!remove[i]) {
            keptGroups.push_back(std::move(object.groups[i]));
            keptRaw.push_back(std::move(object.rawValues[i]));
        }
    }
    object.groups = std::move(keptGroups);
    object.rawValues = std::move(keptRaw);
    return count;
}

std::string upperName(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return name;
}

void noteRationalSplineApproximation(lc::persistence::LossSummary& loss, const DRW_Spline& spline) {
    if ((spline.flags & lc::geo::Spline::RATIONAL) == 0
        && std::none_of(spline.weightlist.begin(), spline.weightlist.end(),
                        [](double weight) { return weight != 1.0; })) return;
    const std::string note = "Rational spline weights are retained, but the displayed curve and geometry operations may be approximate.";
    if (std::find(loss.notes.begin(), loss.notes.end(), note) == loss.notes.end())
        loss.notes.push_back(note);
}

// The kernel's HATCH keeps this immutable source snapshot across ordinary
// entity operations. A simple closed polyline can be replayed through planar
// rigid transforms; other edits take the reported projection path.
class MPolygonSnapshot final : public lc::meta::EntityMetaType {
public:
    struct Vertex { double x, y, bulge; };
    MPolygonSnapshot(const DRW_MPolygon& polygon, const DRW_Hatch& projected,
                     lc::meta::Block_CSPtr projectedBlock, bool sourceIsDwg)
        : block(std::move(projectedBlock)), name(polygon.name), solid(polygon.solid), angle(polygon.angle),
          scale(polygon.scale), layer(projected.layer), lineType(projected.lineType),
          color(projected.color), lineWeight(projected.lWeight), allowElevation(!sourceIsDwg) {
        if (polygon.solid != 1 || polygon.associative != 0 || polygon.isGradient != 0
            || polygon.hstyle != 0 || polygon.hpattern != 1 || polygon.doubleflag != 0
            || !polygon.extData.empty() || !polygon.appData.empty()
            || !polygon.reactorHandles.empty() || polygon.xDictHandle != 0
            || polygon.shadowHandle != 0 || polygon.fullVisualStyleHandle != 0
            || polygon.faceVisualStyleHandle != 0 || polygon.edgeVisualStyleHandle != 0
            || polygon.hasDwgAcDbColorHandle() || polygon.dwgAcDbColorHandle() != 0
            || !std::isfinite(polygon.xDirX) || !std::isfinite(polygon.xDirY)
            || polygon.numProxyGraph != 0 || polygon.hasDataStorageRecord
            || polygon.hasDataStoragePayloadMarker
            || polygon.basePoint.x != 0.0 || polygon.basePoint.y != 0.0
            || !std::isfinite(polygon.basePoint.z)
            || (sourceIsDwg && polygon.basePoint.z != 0.0) || polygon.extPoint.x != 0.0
            || polygon.extPoint.y != 0.0 || polygon.extPoint.z != 1.0
            || polygon.looplist.size() != 1 || !polygon.looplist[0]
            || polygon.looplist[0]->type != 2
            || !polygon.looplist[0]->m_boundaryHandles.empty()
            || polygon.looplist[0]->objlist.size() != 1) return;
        const auto* poly = dynamic_cast<const DRW_LWPolyline*>(
            polygon.looplist[0]->objlist[0].get());
        if (!poly || poly->flags != 1 || poly->vertlist.size() < 3
            || poly->width != 0.0 || poly->elevation != 0.0
            || poly->thickness != 0.0 || poly->extPoint.x != 0.0
            || poly->extPoint.y != 0.0 || poly->extPoint.z != 1.0) return;
        for (const auto& v : poly->vertlist) {
            if (!v || !std::isfinite(v->x) || !std::isfinite(v->y)
                || !std::isfinite(v->bulge) || v->stawidth != 0.0
                || v->endwidth != 0.0) return;
            const double x = v->x + polygon.xDirX;
            const double y = v->y + polygon.xDirY;
            if (!std::isfinite(x) || !std::isfinite(y)) return;
            vertices.push_back({x, y, v->bulge});
        }
        source = std::make_unique<DRW_MPolygon>(polygon);
    }
    const std::string id() const override { return "_LIBDXFRW_MPOLYGON"; }
    const std::string metaTypeID() const override { return id(); }
    bool replayable(const DRW_Hatch& hatch, const lc::entity::Hatch_CSPtr& model,
                    double& outputOffsetX, double& outputOffsetY,
                    double& outputElevation,
                    bool& rewriteBoundary) const {
        if (!source || model->block() != block || hatch.name != name || hatch.solid != solid
            || hatch.angle != angle || hatch.scale != scale
            || hatch.layer != layer || hatch.lineType != lineType
            || hatch.color != color || hatch.lWeight != lineWeight
            || hatch.looplist.size() != 1 || !hatch.looplist[0]
            || hatch.looplist[0]->type != 2
            || hatch.looplist[0]->objlist.size() != 1) return false;
        const auto* poly = dynamic_cast<const DRW_LWPolyline*>(
            hatch.looplist[0]->objlist[0].get());
        if (!poly || poly->flags != 1 || poly->vertlist.size() != vertices.size()
            || poly->width != 0.0 || poly->elevation != 0.0 || poly->thickness != 0.0)
            return false;
        const auto& loops = model->getRegion().loopList();
        if (loops.size() != 1 || loops[0].entities().size() != 1) return false;
        const auto modelPolyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
            loops[0].entities()[0]);
        if (!modelPolyline || modelPolyline->vertex().size() != vertices.size()
            || modelPolyline->extrusionDirection().x() != 0.0
            || modelPolyline->extrusionDirection().y() != 0.0
            || modelPolyline->extrusionDirection().z() != 1.0) return false;
        if (!poly->vertlist[0]) return false;
        const double dx = poly->vertlist[0]->x - vertices[0].x;
        const double dy = poly->vertlist[0]->y - vertices[0].y;
        outputOffsetX = source->xDirX + dx;
        outputOffsetY = source->xDirY + dy;
        if (!std::isfinite(outputOffsetX) || !std::isfinite(outputOffsetY)) return false;
        const auto* original = dynamic_cast<const DRW_LWPolyline*>(
            source->looplist[0]->objlist[0].get());
        if (!original || original->vertlist.size() != vertices.size()) return false;
        bool translation = true;
        outputElevation = modelPolyline->vertex()[0].location().z();
        if (!std::isfinite(outputElevation) || (!allowElevation && outputElevation != 0.0)) return false;
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            if (modelPolyline->vertex()[i].location().z() != outputElevation) return false;
            const auto& v = poly->vertlist[i];
            const auto& raw = original->vertlist[i];
            if (!v || !raw || !std::isfinite(v->x) || !std::isfinite(v->y)
                || v->bulge != vertices[i].bulge || v->stawidth != 0.0
                || v->endwidth != 0.0) return false;
            if (raw->x + outputOffsetX != v->x || raw->y + outputOffsetY != v->y)
                translation = false;
        }
        rewriteBoundary = false;
        if (translation) return true;

        // A solid polygon without references or dormant pattern/gradient data
        // can also be rotated without inventing a transformation for fields
        // the kernel does not model.
        if (source->annotatedBoundary != 0 || source->degenerateLoops != 0
            || source->angle != 0.0 || !source->patternLines.empty()
            || !source->seedPoints.empty() || source->pixelSize != 0.0
            || source->gradReserved != 0 || source->gradAngle != 0.0
            || source->gradShift != 0.0 || source->singleColor != 0
            || source->gradTint != 0.0 || !source->gradName.empty()) return false;
        std::size_t reference = 0;
        double longest = 0.0;
        for (std::size_t i = 1; i < vertices.size(); ++i) {
            const double length = std::hypot(vertices[i].x - vertices[0].x,
                                             vertices[i].y - vertices[0].y);
            if (!std::isfinite(length)) return false;
            if (length > longest) { longest = length; reference = i; }
        }
        if (reference == 0 || longest == 0.0) return false;
        const double ux = (vertices[reference].x - vertices[0].x) / longest;
        const double uy = (vertices[reference].y - vertices[0].y) / longest;
        const double vx0 = poly->vertlist[reference]->x - poly->vertlist[0]->x;
        const double vy0 = poly->vertlist[reference]->y - poly->vertlist[0]->y;
        const double length = std::hypot(vx0, vy0);
        if (!std::isfinite(length) || length == 0.0) return false;
        const double vx = vx0 / length, vy = vy0 / length;
        const double cosine = ux * vx + uy * vy;
        const double sine = ux * vy - uy * vx;
        const double epsilon = std::numeric_limits<double>::epsilon();
        const double scale = std::max({std::numeric_limits<double>::min(), longest, length,
                                       std::abs(vertices[0].x), std::abs(vertices[0].y),
                                       std::abs(poly->vertlist[0]->x),
                                       std::abs(poly->vertlist[0]->y)});
        const double tolerance = 64.0 * epsilon * scale;
        if (!std::isfinite(tolerance) || std::abs(longest - length) > tolerance)
            return false;
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const double sx = vertices[i].x - vertices[0].x;
            const double sy = vertices[i].y - vertices[0].y;
            const double tx = poly->vertlist[i]->x - poly->vertlist[0]->x;
            const double ty = poly->vertlist[i]->y - poly->vertlist[0]->y;
            const double predictedX = cosine * sx - sine * sy;
            const double predictedY = sine * sx + cosine * sy;
            if (!std::isfinite(sx) || !std::isfinite(sy) || !std::isfinite(tx)
                || !std::isfinite(ty) || !std::isfinite(predictedX)
                || !std::isfinite(predictedY)
                || std::abs(predictedX - tx) > tolerance
                || std::abs(predictedY - ty) > tolerance)
                return false;
        }
        const double raw0x = original->vertlist[0]->x;
        const double raw0y = original->vertlist[0]->y;
        outputOffsetX = poly->vertlist[0]->x - (cosine * raw0x - sine * raw0y);
        outputOffsetY = poly->vertlist[0]->y - (sine * raw0x + cosine * raw0y);
        if (!std::isfinite(outputOffsetX) || !std::isfinite(outputOffsetY)) return false;
        // A rounded center may not reconstruct every edited double exactly.
        // Try nearby representable centers before falling back; the bounded
        // search never changes an output vertex or permits approximate replay.
        const auto fitOffset = [&poly](double& offset, bool xAxis) {
            const auto exact = [&poly, xAxis](double candidate) {
                if (!std::isfinite(candidate)) return false;
                for (const auto& vertex : poly->vertlist) {
                    const double point = xAxis ? vertex->x : vertex->y;
                    const double raw = point - candidate;
                    if (!std::isfinite(raw) || raw + candidate != point) return false;
                }
                return true;
            };
            if (exact(offset)) return true;
            double lower = offset, upper = offset;
            for (int i = 0; i < 16; ++i) {
                lower = std::nextafter(lower, -std::numeric_limits<double>::infinity());
                if (exact(lower)) { offset = lower; return true; }
                upper = std::nextafter(upper, std::numeric_limits<double>::infinity());
                if (exact(upper)) { offset = upper; return true; }
            }
            return false;
        };
        if (!fitOffset(outputOffsetX, true) || !fitOffset(outputOffsetY, false)) return false;
        rewriteBoundary = true;
        return true;
    }
    bool canWrite(DRW::Version version) const {
        if (!source || (source->isGradient == 0 && !source->gradColors.empty()))
            return false;
#if !defined(LIBDXFRW_HAS_MPOLYGON_POLYLINE_POLICY)
        // The older bundled codec interprets MPOLYGON groups 70/71 as the
        // HATCH solid/associativity pair, so it cannot safely replay them.
        (void)version;
        return false;
#else
        if (source->dxfVersion != 1) return false;
#endif
        if (version < DRW::AC1018
            && (source->isGradient != 0 || source->gradReserved != 0
                || source->gradAngle != 0.0 || source->gradShift != 0.0
                || source->singleColor != 0 || source->gradTint != 0.0
                || !source->gradName.empty() || !source->gradColors.empty()))
            return false;
        return true;
    }
    static const char* key() { return "_LIBDXFRW_MPOLYGON"; }
    std::unique_ptr<DRW_MPolygon> source;
    lc::meta::Block_CSPtr block;
    std::string name;
    int solid; double angle, scale;
    std::string layer, lineType;
    int color; DRW_LW_Conv::lineWidth lineWeight;
    bool allowElevation;
    std::vector<Vertex> vertices;
};
} // namespace


const std::map<int, lc::Units> DXFimpl::_dxfToLCUnits = {
    {0, lc::Units::None},
    {1, lc::Units::Inch},
    {2, lc::Units::Foot},
    {3, lc::Units::Mile},
    {4, lc::Units::Millimeter},
    {5, lc::Units::Centimeter},
    {6, lc::Units::Meter},
    {7, lc::Units::Kilometer},
    {8, lc::Units::Microinch},
    {9, lc::Units::Mil},
    {10, lc::Units::Yard},
    {11, lc::Units::Angstrom},
    {12, lc::Units::Nanometer},
    {13, lc::Units::Micron},
    {14, lc::Units::Decimeter},
    {15, lc::Units::Decameter},
    {16, lc::Units::Hectometer},
    {17, lc::Units::Gigameter},
    {18, lc::Units::Astro},
    {19, lc::Units::Lightyear},
    {20, lc::Units::Parsec},
};

const std::map<lc::Units, int> DXFimpl::_lcUnitsToDXF = lc::tools::MapHelper::reverse(_dxfToLCUnits);


DXFimpl::DXFimpl(std::shared_ptr<lc::storage::Document> document, lc::operation::Builder_SPtr builder) :
    _document(document),
    _builder(std::move(builder)),
    _entityBuilder(std::make_shared<lc::operation::EntityBuilder>(document)),
    _currentBlock(nullptr),
    dxfW(nullptr) {
    _builder->append(_entityBuilder);
}

inline int DXFimpl::widthToInt(double wid) const {
    for (int i = 0; i < 24; i++) {
        if (lc::persistence::FileHelpers::intToLW(i).width() == wid) {
            return i;
        }
    }

    return -1;
}

namespace {

/**
 * Read a header variable without consuming it.
 *
 * DRW_Header::getStr() and friends delete the variant and erase the entry, and
 * they are non-const, so they cannot be used on the const header a callback is
 * handed. Reading `vars` directly leaves the header intact for anyone else who
 * looks at it. The '$'-less spelling is tried too, exactly as the library's own
 * lookup does: DWG headers store the names without it.
 */
const DRW_Variant* headerVar(const DRW_Header& header, const std::string& key) {
    auto it = header.vars.find(key);
    if (it == header.vars.end()) {
        it = header.vars.find(!key.empty() && key[0] == '$' ? key.substr(1) : "$" + key);
    }
    return it == header.vars.end() ? nullptr : it->second;
}

bool headerStr(const DRW_Header& header, const std::string& key, std::string& out) {
    const DRW_Variant* var = headerVar(header, key);
    if (var == nullptr || var->type() != DRW_Variant::STRING || var->content.s == nullptr) {
        return false;
    }
    out = *var->content.s;
    return true;
}

bool headerInt(const DRW_Header& header, const std::string& key, int& out) {
    const DRW_Variant* var = headerVar(header, key);
    if (var == nullptr || var->type() != DRW_Variant::INTEGER) {
        return false;
    }
    out = var->content.i;
    return true;
}

bool headerDouble(const DRW_Header& header, const std::string& key, double& out) {
    const DRW_Variant* var = headerVar(header, key);
    if (var == nullptr || var->type() != DRW_Variant::DOUBLE) {
        return false;
    }
    out = var->content.d;
    return true;
}

bool headerCoord(const DRW_Header& header, const std::string& key, DRW_Coord& out) {
    const DRW_Variant* var = headerVar(header, key);
    if (var == nullptr || var->type() != DRW_Variant::COORD || var->content.v == nullptr) {
        return false;
    }
    out = *var->content.v;
    return true;
}

}  // namespace

// Until now this was an empty override, so everything the HEADER section says
// about the drawing was discarded -- including the one thing File::open needed
// to record the file's revision, which it was instead guessing from the text
// codec.
void DXFimpl::addHeader(const DRW_Header* data) {
    guarded("HEADER", nullptr, [&] {
        LOG_TRACE << "addHeader";

        if (data == nullptr) {
            return;
        }

        headerStr(*data, "$ACADVER", _header.acadVersion);

        _header.hasInsUnits = headerInt(*data, "$INSUNITS", _header.insUnitsCode);
        if (_header.hasInsUnits) {
            _header.units = numberToUnit(_header.insUnitsCode);
        }

        _header.hasMeasurement = headerInt(*data, "$MEASUREMENT", _header.measurement);
        headerDouble(*data, "$LTSCALE", _header.lineTypeScale);

        // Extents are a pair or they are nothing: half a bounding box describes no
        // region, and a consumer that trusted one half would read the other as the
        // origin.
        DRW_Coord extMin;
        DRW_Coord extMax;
        if (headerCoord(*data, "$EXTMIN", extMin) && headerCoord(*data, "$EXTMAX", extMax)) {
            _header.extMin = coord(extMin);
            _header.extMax = coord(extMax);
            _header.hasExtents = true;
        }

        LOG_TRACE << "header $ACADVER=" << _header.acadVersion
                  << " $INSUNITS=" << _header.insUnitsCode
                  << " $MEASUREMENT=" << _header.measurement;
    });
}

void DXFimpl::recordFailure(const char* recordKind, const DRW_Entity* entity, const char* reason) {
    ImportFailure failure;
    failure.recordKind = recordKind;
    failure.reason = reason == nullptr ? "" : reason;

    if (entity != nullptr) {
        failure.handle = entity->handle;
        failure.layer = entity->layer;

        // The block it belongs to, by the name the drawing uses: the handle
        // alone is no help to a user looking for what went missing.
        const auto block = _handleBlock.find(entity->parentHandle);
        if (block != _handleBlock.end() && block->second != nullptr) {
            failure.block = block->second->name();
        } else if (_currentBlock != nullptr) {
            failure.block = _currentBlock->name();
        }
    }

    LOG_ERROR << "Could not import " << failure.recordKind
              << " (handle " << failure.handle << ", layer " << failure.layer << "): "
              << failure.reason;

    _failures.push_back(std::move(failure));
}

void DXFimpl::setBlock(const int handle) {
    LOG_TRACE << "setBlock " << handle;
}

void DXFimpl::addViewport(const DRW_Viewport& data) {
    guarded("VIEWPORT", &data, [&] {
        LOG_TRACE << "addViewport ";
    });
}

void DXFimpl::addVport(const DRW_Vport& data) {
    guarded("VPORT", nullptr, [&] {
        LOG_TRACE << "addVport ";
    });
}

void DXFimpl::addBlock(const DRW_Block& data) {
    guarded("BLOCK", &data, [&] {
        LOG_TRACE << "addBlock " << data.name;
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        if (upperName(data.name) == "*PAPER_SPACE0") _sourceHasPaperSpace0 = true;
#endif

        _currentBlock = nullptr;

        auto base = coord(data.basePoint);
        std::string blockName = data.name;
        bool legacyPaper = false;
        // R12 uppercases names on the wire; R14 may use either spelling.
        // Match the kernel defaults but retain the source lookup below.
        if (_header.acadVersion.empty() || _header.acadVersion == "AC1009" ||
            _header.acadVersion == "AC1014") {
            const auto upper = upperName(blockName);
            if (upper == "*PAPER_SPACE") { blockName = "*Paper_Space"; legacyPaper = true; }
            else if (upper == "*PAPER_SPACE0") { blockName = "*Paper_Space0"; legacyPaper = true; }
        }

        auto appData = data.appData;
        auto it = appData.begin();
        std::string appName;

        while(it != appData.end()) {
            if(!it->empty()) {
                appName = *(it->begin()->content.s);

                if(appName == APP_NAME) {
                    break;
                }
            }

            it++;
        }

        if(it != appData.end() && it->size() >= 3) {
            auto it2 = it->begin();

            it2++;
            auto pluginName = *(it2->content.s);

            it2++;
            auto entityName = *(it2->content.s);

            it2++;

            std::map<std::string, std::string> params;
            while(it2 != it->end()) {
                auto key = *(it2->content.s);

                it2++;

                if(it2 == it->end()) {
                    break;
                }

                auto value = *(it2->content.s);
                params[key] = value;
                it2++;
            }

            // Under the drawing's own block name: the INSERTs that place this
            // custom entity reference it by that name, and CustomEntityStorage's
            // other constructor invents one, so every such INSERT used to end up
            // pointing at a block nothing defined -- which is why no plugin was
            // ever asked to rebuild a custom entity read back from a file.
            _currentBlock = std::make_shared<lc::meta::CustomEntityStorage>(
                blockName, pluginName, entityName, base, params);
        }

        if(_currentBlock == nullptr) {
            // Bit 1 of BLOCK's code 70: the source generated this block itself.
            // The writer regenerates the dimension geometry blocks and has to
            // leave the rest alone, and this flag is the only honest way to
            // tell them apart -- the *D<n> naming convention is a convention,
            // not a guarantee.
            //
            // The DWG reader does not fill flags in, so a DWG's own anonymous
            // blocks arrive unmarked; _sourceIsDwg stands in for the bit there,
            // where the convention is all there is.
            const bool anonymous = (data.flags & 1) != 0
                || (_sourceIsDwg && isAnonymousDimensionBlockName(data.name));
            _currentBlock = std::make_shared<lc::meta::Block>(blockName, base, anonymous);
        }
        // Blocks are parsed before the top-level Builder executes, so the
        // document's name lookup does not yet include this definition. Keep
        // the parsed block indexed by its file name while the read is active.
        _blocks[data.name] = _currentBlock;
        const auto existing = legacyPaper ? _document->blockByName(blockName) : nullptr;
        if (existing) {
            // Preserve the imported block's base and anonymous state as well as
            // its children; AddBlock alone leaves the kernel default in place.
            _builder->append(std::make_shared<lc::operation::ReplaceBlock>(_document, existing, _currentBlock));
        } else {
            _builder->append(std::make_shared<lc::operation::AddBlock>(_document, _currentBlock));
        }

        // May need to check if the block already exists: not sure
        _handleBlock.insert(std::pair<int, lc::meta::Block_CSPtr>(data.parentHandle, _currentBlock));
    });
}

void DXFimpl::endBlock() {
    LOG_TRACE << "endBlock";
    _currentBlock = nullptr;
}

void DXFimpl::addSolid(const DRW_Solid& data) {
    guarded("SOLID", &data, [&] {
        LOG_TRACE << "addSolid";

        // A SOLID is a filled triangle or quadrilateral. LibreCAD has no filled
        // polygon, but it has a solid-filled Hatch, which draws the same thing
        // -- so a record that used to be counted and dropped now arrives as
        // what it depicts.
        //
        // The corner order is the part worth getting right: DXF numbers a
        // SOLID's points 10, 11, 12, 13 in a bow-tie, so the boundary runs
        // first -> second -> fourth -> third. Taking them in numeric order
        // draws a crossed quadrilateral instead of the filled one.
        const auto first = coord(data.basePoint);
        const auto second = coord(data.secPoint);
        const auto third = coord(data.thirdPoint);
        const auto fourth = coord(data.fourPoint);

        std::vector<lc::geo::Coordinate> corners{first, second, fourth, third};

        // A triangular SOLID repeats its last corner; a zero-length edge would
        // make a degenerate boundary, so collapse the repeat instead.
        if (third.x() == fourth.x() && third.y() == fourth.y() && third.z() == fourth.z()) {
            corners.pop_back();
        }

        auto layer = getLayer(data);
        std::vector<lc::entity::CADEntity_CSPtr> boundary;
        for (std::size_t i = 0; i < corners.size(); i++) {
            const auto& from = corners[i];
            const auto& to = corners[(i + 1) % corners.size()];
            if (from.x() == to.x() && from.y() == to.y() && from.z() == to.z()) {
                continue;  // a degenerate edge is no edge
            }

            lc::builder::LineBuilder edge;
            edge.setStart(from);
            edge.setEnd(to);
            edge.setLayer(layer);
            boundary.push_back(edge.build());
        }

        if (boundary.size() < 3) {
            LOG_WARNING << "Skipping SOLID with fewer than three distinct corners";
            recordLoss("SOLID");
            return;
        }

        lc::geo::Region region;
        region.addLoop(lc::geo::Loop(boundary));

        auto hatch = std::make_shared<lc::entity::Hatch>(layer, getMetaInfo(data), getBlock(data));
        hatch->setPatternName("SOLID");
        hatch->setSolid(true);
        hatch->setAngle(0.0);
        hatch->setScale(1.0);
        hatch->setRegion(region);

        deliver(hatch);
    });
}

void DXFimpl::addLine(const DRW_Line& data) {
    guarded("LINE", &data, [&] {
        LOG_TRACE << "addLine";
        lc::builder::LineBuilder builder;

        builder.setMetaInfo(getMetaInfo(data));
        builder.setBlock(getBlock(data));
        builder.setLayer(getLayer(data));
        builder.setStart(coord(data.basePoint));
        builder.setEnd(coord(data.secPoint));

        LOG_TRACE << "Block:" << builder.block();
        deliver(builder.build());
    });
}

void DXFimpl::addCircle(const DRW_Circle& data) {
    guarded("CIRCLE", &data, [&] {
        LOG_TRACE << "addCircle";
        // CircleBuilder::checkValues throws for a negative radius (and geo::Circle
        // throws again).  The throw travels out through libdxfrw's callback and out
        // of File::open, which no caller guards -- lcUI/cadmdichild.cpp:107 calls it
        // bare -- so one bad record terminated the process.  Drop the record.
        // The negated comparison also rejects NaN.
        if (!(data.radious >= 0.0)) {
            LOG_ERROR << "Skipping CIRCLE with unusable radius " << data.radious;
            return;
        }

        lc::builder::CircleBuilder builder;

        builder.setMetaInfo(getMetaInfo(data));
        builder.setLayer(getLayer(data));
        builder.setCenter(coord(data.basePoint));
        builder.setRadius(data.radious);
        builder.setBlock(getBlock(data));

        deliver(builder.build());
    });
}

void DXFimpl::addArc(const DRW_Arc& data) {
    guarded("ARC", &data, [&] {
        LOG_TRACE << "addArc";
        // geo::Arc's constructor throws std::runtime_error("Invalid radius") for
        // radius <= 0 (lckernel/cad/geometry/geoarc.cpp:14-16) and ArcBuilder does
        // not check it, so an ARC with 40=0 aborted the whole open.  The negated
        // comparison also rejects NaN.
        if (!(data.radious > 0.0)) {
            LOG_ERROR << "Skipping ARC with unusable radius " << data.radious;
            return;
        }

        lc::builder::ArcBuilder builder;

        builder.setMetaInfo(getMetaInfo(data));
        builder.setLayer(getLayer(data));
        builder.setBlock(getBlock(data));
        builder.setCenter(coord(data.basePoint));
        builder.setRadius(data.radious);
        builder.setStartAngle(data.staangle);
        builder.setEndAngle(data.endangle);
        builder.setIsCCW((bool) data.isccw);

        deliver(builder.build());
    });
}

void DXFimpl::addEllipse(const DRW_Ellipse& data) {
    guarded("ELLIPSE", &data, [&] {
        LOG_TRACE << "addEllipse";
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);
        auto layer = getLayer(data);

        auto secPoint = coord(data.secPoint);
        auto lcEllipse = std::make_shared<lc::entity::Ellipse>(coord(data.basePoint),
                         secPoint,
                         secPoint.magnitude() * data.ratio,
                         data.staparam,
                         data.endparam,
                         data.isccw,
                         layer,
                         mf,
                         getBlock(data)
                                                              );

        deliver(lcEllipse);
    });
}

void DXFimpl::addLayer(const DRW_Layer& data) {
    guarded("LAYER", nullptr, [&] {
        LOG_TRACE << "addLayer " << data.name;
        const int sourceAci = data.color < 0 && data.color >= -255 ? -data.color : data.color;
        auto col = icol.intToColor(sourceAci);

        if (col == nullptr) {
            col = icol.intToColor(255);
        }

        auto lw = getLcLineWidth<lc::meta::MetaLineWidthByValue>(data.lWeight);

        if (lw == nullptr) {
            lw = getLcLineWidth<lc::meta::MetaLineWidthByValue>(DRW_LW_Conv::lineWidth::width00);
        }

        auto lp = _document->linePatternByName(data.lineType);
        auto isFrozen = (bool) ((unsigned int) data.flags & 1u);

        const auto color = data.color24 >= 0 && data.color24 <= 0xFFFFFF
            ? lc::Color((data.color24 >> 16) & 0xFF, (data.color24 >> 8) & 0xFF, data.color24 & 0xFF)
            : col->color();
        auto layer = std::make_shared<lc::meta::Layer>(data.name, lw->width(), color, lp, isFrozen, data.color < 0, (data.flags & 4) != 0);
        if (!data.name.empty() && data.name[0] != '*') {
            _layers[upperName(data.name)] = layer;
            _preserved.sourceLayerColors[data.name] = {layer, data.color, data.color24, data.colorName};
        }
        _preserved.sourceLayerWeights[data.name] = {
            lw->width(), data.lWeight == DRW_LW_Conv::widthDefault};
        // If a layer starts with a * it's a special layer we don't process yet
        if(data.name == "0") {
            auto al = std::make_shared<lc::operation::ReplaceLayer>(_document, _document->layerByName("0"), layer);
            _builder->append(al);
        }
        else if (data.name.length() > 0 && (data.name.compare(0,1,"*") != 0)) {
            auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
            _builder->append(al);
        }
    });
}

void DXFimpl::addSpline(const DRW_Spline* data) {
    guarded("SPLINE", data, [&] {
        LOG_TRACE << "addSpline";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);

        // The knot vector is stored as the file gives it. It used to be trimmed by
        // one knot at each end, which made every imported spline unwritable: the
        // library requires knotCount == controlCount + degree + 1 and refuses the
        // whole file otherwise. Nothing consumed the trimmed form either --
        // geo::Spline::populateCurve builds a clamped B-spline from the control
        // points and never reads the stored knots.
        auto lcSpline = std::make_shared<lc::entity::Spline>(coords(data->controllist),
                        data->knotslist,
                        coords(data->fitlist),
                        data->degree,
                        false,
                        data->tolfit,
                        data->tgStart.x, data->tgStart.y, data->tgStart.z,
                        data->tgEnd.x, data->tgEnd.y, data->tgEnd.z,
                        data->normalVec.x, data->normalVec.y, data->normalVec.z,
                        static_cast<lc::geo::Spline::splineflag>(data->flags),
                        layer,
                        mf,
                        getBlock(*data), data->weightlist
                                                            );
        noteRationalSplineApproximation(_loss, *data);
        deliver(lcSpline);
    });
}

void DXFimpl::addText(const DRW_Text& data) {
    guarded("TEXT", &data, [&] {
        LOG_TRACE << "addText";
        auto layer = getLayer(data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);
        // A TEXT record carries no escape language -- only the %% forms it
        // shares with MTEXT, which LibreCAD has never decoded either.
        auto lcText = std::make_shared<lc::entity::Text>(coord(data.basePoint),
                      lc::persistence::textToPlain(data.text), data.height,
                      data.angle * M_PI / 180, data.style,
                      lc::TextConst::DrawingDirection(data.textgen),
                      lc::TextConst::HAlign(data.alignH),
                      lc::TextConst::VAlign(data.alignV),
                      layer,
                      mf,
                      getBlock(data)
                                                        );

        deliver(lcText);
    });
}

void DXFimpl::addPoint(const DRW_Point& data) {
    guarded("POINT", &data, [&] {
        LOG_TRACE << "addPoint";
        auto layer = getLayer(data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);
        auto lcPoint = std::make_shared<lc::entity::Point>(coord(data.basePoint),
                       layer,
                       mf,
                       getBlock(data)
                                                          );

        deliver(lcPoint);
    });
}

void DXFimpl::addDimAlign(const DRW_DimAligned* data) {
    guarded("DIMENSION (aligned)", data, [&] {
        LOG_TRACE << "addDimAlign";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);
        auto lcDimAligned = std::make_shared<lc::entity::DimAligned>(
                                coord(data->getDefPoint()),
                                coord(data->getTextPoint()),
                                static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                                data->getDir(),
                                data->getTextLineFactor(),
                                static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                                data->getText(),
                                coord(data->getDef1Point()),
                                coord(data->getDef2Point()),
                                layer,
                                mf,
                                getBlock(*data)
                            );

        deliver(lcDimAligned);
    });
}

void DXFimpl::addDimLinear(const DRW_DimLinear* data) {
    guarded("DIMENSION (linear)", data, [&] {
        LOG_TRACE << "addDimLinear";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);
        auto lcDimLinear = std::make_shared<lc::entity::DimLinear>(
                               coord(data->getDefPoint()),
                               coord(data->getTextPoint()),
                               static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                               data->getDir(),
                               data->getTextLineFactor(),
                               static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                               data->getText(),
                               coord(data->getDef1Point()),
                               coord(data->getDef2Point()),
                               // Groups 50 and 52 are in degrees (libdxfrw also
                               // converts a DWG's radians to them); the kernel's
                               // angles are radians.
                               data->getAngle() * M_PI / 180,
                               data->getOblique() * M_PI / 180,
                               layer,
                               mf,
                               getBlock(*data)
                           );

        deliver(lcDimLinear);
    });
}

void DXFimpl::addDimRadial(const DRW_DimRadial* data) {
    guarded("DIMENSION (radial)", data, [&] {
        LOG_TRACE << "addDimRadial";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);
        auto  lcDimRadial = std::make_shared<lc::entity::DimRadial>(
                                coord(data->getCenterPoint()),
                                coord(data->getTextPoint()),
                                static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                                data->getDir(),
                                data->getTextLineFactor(),
                                static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                                data->getText(),
                                coord(data->getDiameterPoint()),
                                data->getLeaderLength(),
                                layer,
                                mf,
                                getBlock(*data)
                            );

        deliver(lcDimRadial);
    });
}

void DXFimpl::addDimDiametric(const DRW_DimDiametric* data) {
    guarded("DIMENSION (diametric)", data, [&] {
        LOG_TRACE << "addDimDiametric";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);
        auto lcDimDiametric = std::make_shared<lc::entity::DimDiametric>(
                                  coord(data->getDiameter1Point()),
                                  coord(data->getTextPoint()),
                                  static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                                  data->getDir(),
                                  data->getTextLineFactor(),
                                  static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                                  data->getText(),
                                  coord(data->getDiameter2Point()),
                                  data->getLeaderLength(),
                                  layer,
                                  mf,
                                  getBlock(*data)
                              );

        deliver(lcDimDiametric);
    });
}

void DXFimpl::addDimAngular(const DRW_DimAngular* data) {
    guarded("DIMENSION (angular)", data, [&] {
        LOG_TRACE << "addDimAngular";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);
        auto lcDimAngular = std::make_shared<lc::entity::DimAngular>(
                                coord(data->getDefPoint()),
                                coord(data->getTextPoint()),
                                static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                                data->getDir(),
                                data->getTextLineFactor(),
                                static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                                data->getText(),
                                coord(data->getFirstLine1()),
                                coord(data->getFirstLine2()),
                                coord(data->getSecondLine1()),
                                coord(data->getSecondLine2()),
                                layer,
                                mf,
                                getBlock(*data)
                            );

        deliver(lcDimAngular);
    });
}

void DXFimpl::addDimAngular3P(const DRW_DimAngular3p* data) {
    guarded("DIMENSION (3-point angular)", data, [&] {
        LOG_WARNING << "Dropping DIMENSION (3-point angular): no kernel entity for it";
        recordLoss("DIMENSION (3-point angular)");
    });
}

void DXFimpl::addDimOrdinate(const DRW_DimOrdinate* data) {
    guarded("DIMENSION (ordinate)", data, [&] {
        LOG_TRACE << "addDimOrdinate";
        auto layer = getLayer(*data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*data);
        // Which axis is measured is bit 6 of group 70, which libdxfrw keeps
        // as read and has no accessor for. Its DWG reader sets the same bit
        // from the DWG's own flag, so this covers both.
        auto lcDimOrdinate = std::make_shared<lc::entity::DimOrdinate>(
                                 coord(data->getOriginPoint()),
                                 coord(data->getTextPoint()),
                                 static_cast<lc::TextConst::AttachmentPoint>(data->getAlign()),
                                 data->getDir(),
                                 data->getTextLineFactor(),
                                 static_cast<lc::TextConst::LineSpacingStyle>(data->getTextLineStyle()),
                                 data->getText(),
                                 coord(data->getFirstLine()),
                                 coord(data->getSecondLine()),
                                 (data->type & 64) != 0,
                                 layer,
                                 mf,
                                 getBlock(*data)
                             );

        deliver(lcDimOrdinate);
    });
}

void DXFimpl::addLWPolyline(const DRW_LWPolyline& data) {
    guarded("LWPOLYLINE", &data, [&] {
        LOG_TRACE << "addLWPolyline";
        auto layer = getLayer(data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);

        std::vector<lc::entity::LWVertex2D> points;
        for (const auto& i : data.vertlist) {
            points.emplace_back(lc::geo::Coordinate(i->x, i->y), i->bulge, i->stawidth, i->endwidth);
        }

        auto isCLosed = (unsigned int) data.flags & 0x01u;
        auto lcLWPolyline = std::make_shared<lc::entity::LWPolyline>(
                                points,
                                data.width,
                                data.elevation,
                                data.thickness,
                                isCLosed,
                                coord(data.extPoint),
                                layer,
                                mf,
                                getBlock(data)
                            );

        deliver(lcLWPolyline);
    });
}

//Handle polyline as lwpolyline
void DXFimpl::addPolyline(const DRW_Polyline& data) {
    guarded("POLYLINE", &data, [&] {
        LOG_TRACE << "addPolyline";
        auto layer = getLayer(data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);

        std::vector<lc::entity::LWVertex2D> points;
        for (const auto& i : data.vertlist) {
            points.emplace_back(coord(i->basePoint), i->bulge, i->stawidth, i->endwidth);
        }

        auto isCLosed = (unsigned int) data.flags & 0x01u;

        auto lcLWPolyline = std::make_shared<lc::entity::LWPolyline>(
                                points,
                                0.0,
                                0.0,
                                0.0,
                                isCLosed,
                                coord(data.extPoint),
                                layer,
                                mf,
                                getBlock(data)
                            );

        deliver(lcLWPolyline);
    });
}

std::shared_ptr<lc::entity::MText> DXFimpl::makeMText(
    const DRW_MText& data, const DRW_Entity& owner) {
    auto layer = getLayer(owner);
    std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(owner);
    lc::TextConst::HAlign halign;
    lc::TextConst::VAlign valign;
    //lc::TextConst::AttachmentPoint attachmentPoint = lc::TextConst::AttachmentPoint(data.textgen);
    lc::TextConst::DrawingDirection drawingDir;

    switch (data.textgen % 3) {
    default:
    case 1:
        halign = lc::TextConst::HAlign::HALeft;
        break;
    case 2:
        halign = lc::TextConst::HAlign::HACenter;
        break;
    case 0:
        halign = lc::TextConst::HAlign::HARight;
        break;
    }

    switch ((int)(std::ceil(data.textgen / 3.0))) {
    default:
    case 1:
        valign = lc::TextConst::VAlign::VATop;
        break;
    case 2:
        valign = lc::TextConst::VAlign::VAMiddle;
        break;
    case 3:
        valign = lc::TextConst::VAlign::VABottom;
        break;
    }

    if (data.alignH == 1) {
        drawingDir = lc::TextConst::DrawingDirection::Backward;
    }
    else if (data.alignH == 3) {
        drawingDir = lc::TextConst::DrawingDirection::UpsideDown;
    }
    else {
        drawingDir = lc::TextConst::DrawingDirection::None;
    }

    // Code 73, line spacing style: 1 = at least, 2 = exact.  Neither the
    // enum's own ordering nor a cast gets this right, and real files carry
    // values the specification does not define -- over 150 drawings here,
    // 269 of 877 MTEXT records say 73 = 0 and 4 omit it.  Anything that is
    // not 2 takes the DXF default.
    const auto lineSpacingStyle = (data.linespacingStyle == 2)
        ? lc::TextConst::LineSpacingStyle::Exact
        : lc::TextConst::LineSpacingStyle::AtLeast;

    // Code 44, line spacing factor, 0.25 to 4.  libdxfrw defaults it to 1
    // when the group is absent, which is also the DXF default, so an
    // out-of-range value is the only thing worth refusing.
    const double lineSpacingFactor =
        (data.interlin >= 0.25 && data.interlin <= 4.0) ? data.interlin : 1.0;

    // Code 72, drawing direction: 1 left-to-right, 3 top-to-bottom, 5 by
    // style.  libdxfrw routes it into DRW_Text::alignH, whose type has
    // nothing to do with it.  Do not cast: in the same 150 drawings, 456
    // of 877 MTEXT records carry 72 = 2, which is not a defined value.
    // An undefined direction becomes ByStyle, which is what the file is
    // effectively asking for.
    lc::TextConst::MTextDrawingDirection mtextDirection;
    switch (static_cast<int>(data.alignH)) {
    case lc::TextConst::MTextDrawingDirection::LeftToRight:
        mtextDirection = lc::TextConst::MTextDrawingDirection::LeftToRight;
        break;
    case lc::TextConst::MTextDrawingDirection::TopToBottom:
        mtextDirection = lc::TextConst::MTextDrawingDirection::TopToBottom;
        break;
    default:
        mtextDirection = lc::TextConst::MTextDrawingDirection::ByStyle;
        break;
    }

    // Code 41, the reference rectangle width.  libdxfrw carries it in
    // DRW_Text::widthscale, which is TEXT's width FACTOR -- a different
    // quantity in the same member -- and DRW_Text's constructor defaults
    // it to 1, so an MTEXT with no group 41 is indistinguishable from one
    // that asks to wrap at one drawing unit.  Every one of the 877 MTEXT
    // records in the corpus carries group 41 and none of them carries 1.0,
    // so the ambiguity is not one real files reach.
    const double width = (data.widthscale > 0.0) ? data.widthscale : 0.0;

    auto lcMText = std::make_shared<lc::entity::MText>(coord(data.basePoint),
                  lc::persistence::mtextToPlain(data.text), data.height,
                  data.angle * M_PI / 180, data.style,
                  lc::TextConst::DrawingDirection(drawingDir),
                  lc::TextConst::HAlign(halign),
                  lc::TextConst::VAlign(valign),
                  false,
                  false,
                  false,
                  false,
                  width,
                  mtextDirection,
                  lineSpacingFactor,
                  lineSpacingStyle,
                  layer,
                  mf,
                  getBlock(owner)
                                                    );

    // MTEXT columns and background fill are XDATA, not AcDbMText group
    // codes, and libdxfrw has already captured them into extData. Keep
    // them on the document's shelf so a save can hand them back: this is
    // the one place the adapter carries something it does not model, and
    // nothing here interprets a single variant.
    if (!data.extData.empty()) {
        PreservedRecords::MTextExtendedData carried;
        carried.text = lcMText->text_value();
        carried.data = data.extData;
        _preserved.mtextExtendedData.emplace(lcMText->id(), std::move(carried));
    }

    return lcMText;
}

void DXFimpl::addMText(const DRW_MText& data) {
    guarded("MTEXT", &data, [&] {
        LOG_TRACE << "addMText";
        deliver(makeMText(data, data));
    });
}

void DXFimpl::addHatch(const DRW_Hatch* data) {
    guarded("HATCH", data, [&] {
        // Loop->objlist contains the 3 entities (copied) that define the hatch areas are the entities selected during hatch
        // loopList seems to contain the same entities, why??
        LOG_TRACE << "addHatch ";
        auto layer = getLayer(*data);
        auto mf = getMetaInfo(*data);
        const auto* polygon = dynamic_cast<const DRW_MPolygon*>(data);
        if (polygon && !mf) mf = lc::meta::MetaInfo::create();
        // MPOLYGON stores boundary points in offset space. The offset is a
        // point displacement, so it applies to points but not axis vectors or
        // spline tangents. Ordinary HATCH boundaries have no such offset.
        const double offsetX = polygon ? polygon->xDirX : 0.0;
        const double offsetY = polygon ? polygon->xDirY : 0.0;
        const auto addOffset = [polygon](double value, double offset) {
            if (!polygon) return value;
            const double mapped = value + offset;
            if (polygon && !std::isfinite(mapped))
                throw std::overflow_error("MPOLYGON boundary offset is not finite");
            return mapped;
        };
        const auto pointInOffsetPlane = [offsetX, offsetY, addOffset](const DRW_Coord& point) {
            return lc::geo::Coordinate(addOffset(point.x, offsetX),
                                       addOffset(point.y, offsetY), point.z);
        };
        // Default-normal DXF HATCH and MPOLYGON polyline paths lie at
        // the entity's OCS elevation; their path coordinates have no Z.
        // Leave tilted planes and DWG MPOLYGON projection on existing paths.
        const bool liftBoundaryPlane = (!polygon || !_sourceIsDwg) && data->extPoint.x == 0.0
            && data->extPoint.y == 0.0 && data->extPoint.z == 1.0
            && std::isfinite(data->basePoint.z);
        const double hatchPolylinePlaneZ = liftBoundaryPlane ? data->basePoint.z : 0.0;
        const auto boundaryPointInPlane = [&pointInOffsetPlane, liftBoundaryPlane, hatchPolylinePlaneZ](const DRW_Coord& point) {
            const auto mapped = pointInOffsetPlane(point);
            return liftBoundaryPlane ? lc::geo::Coordinate(mapped.x(), mapped.y(), hatchPolylinePlaneZ) : mapped;
        };
        const auto pointsInOffsetPlane = [this, offsetX, offsetY, addOffset, liftBoundaryPlane, hatchPolylinePlaneZ](const auto& points) {
            auto mapped = coords(points);
            for (auto& point : mapped)
                point = lc::geo::Coordinate(addOffset(point.x(), offsetX),
                                            addOffset(point.y(), offsetY),
                                            liftBoundaryPlane ? hatchPolylinePlaneZ : point.z());
            return mapped;
        };
        lc::geo::Region reg;
        auto lcHatch = std::make_shared<lc::entity::Hatch>(   layer,
                       mf,
                       getBlock(*data)
                                                          );
        if (polygon) {
            DRW_Hatch projected;
            getEntityAttributes(&projected, lcHatch);
            mf->add(std::make_shared<MPolygonSnapshot>(*polygon, projected, lcHatch->block(), _sourceIsDwg));
        }
        lcHatch->setPatternName(data->name);
        int solidFlag = data->solid;
#if !defined(LIBDXFRW_HAS_MPOLYGON_POLYLINE_POLICY)
        // Older libdxfrw applies HATCH's group 70/71 interpretation to DXF
        // MPOLYGON. Its DWG parser already delivers the actual solid flag.
        if (polygon && !_sourceIsDwg) solidFlag = polygon->associative;
#endif
        lcHatch->setSolid(solidFlag);
        // MPOLYGON's fill color is independent of its common entity color.
        // Keep it separate so the snapshot and DXF group 62 remain unchanged.
        if (polygon && !_sourceIsDwg && solidFlag && !polygon->isGradient && polygon->gradColors.empty() &&
            polygon->fillColorAci > 0 && polygon->fillColorAci < 256 &&
            polygon->fillColorRgb < 0 && polygon->fillColorName.empty()) {
            const auto fill = icol.intToColor(polygon->fillColorAci);
            if (fill) mf->add(std::make_shared<lc::meta::MetaFillColor>(fill->color(), polygon->fillColorAci));
        }
        LOG_TRACE << "name " << data->name;
        LOG_TRACE << "solid " << solidFlag;
        if(!solidFlag) {
            //Load pattern from dxf
            lcHatch->setPattern(lc::persistence::PatternProvider::Instance()->getPattern(data->name));
        }
        LOG_TRACE << "associative " << data->associative;           /*!< associativity, code 71, associatve=1, non-assoc.=0 */
        //lcHatch->setHatchStyle(data->hstyle);
        //lcHatch->setHatchPattern(data->hpattern);
        LOG_TRACE << "double flag " << data->doubleflag;            /*!< hatch pattern double flag, code 77, double=1, single=0 */
        LOG_TRACE << "loopsnum " <<data->loopsnum;              /*!< namber of boundary paths (loops), code 91 */
        lcHatch->setAngle(data->angle);
        lcHatch->setScale(data->scale);
        LOG_TRACE << "deflines " << data->deflines;              /*!< number of pattern definition lines, code 78 */
        for (auto x : data->looplist) {
            std::vector<lc::entity::CADEntity_CSPtr> loopData;
            for(auto k : x->objlist) {
                if(k->eType == DRW::ETYPE::LWPOLYLINE) { //done
                    auto data = std::dynamic_pointer_cast<DRW_LWPolyline>(k);
                    LOG_TRACE << "Polyline";
                    std::vector<lc::entity::LWVertex2D> points;
                    for (const auto& i : data->vertlist) {
                        points.emplace_back(lc::geo::Coordinate(addOffset(i->x, offsetX),
                                                                addOffset(i->y, offsetY),
                                                                hatchPolylinePlaneZ),
                                            i->bulge, i->stawidth, i->endwidth);
                    }
                    auto isCLosed = (unsigned int) data->flags & 0x01u;
                    auto lcLWPolyline = std::make_shared<lc::entity::LWPolyline>(
                                            points,
                                            data->width,
                                            data->elevation,
                                            data->thickness,
                                            isCLosed,
                                            coord(data->extPoint),
                                            layer
                                        );
                    loopData.push_back(lcLWPolyline);
                } else if(k->eType == DRW::ETYPE::LINE) { //done
                    auto data = std::dynamic_pointer_cast<DRW_Line>(k);
                    LOG_TRACE << "line";
                    lc::builder::LineBuilder builder;
                    builder.setStart(boundaryPointInPlane(data->basePoint));
                    builder.setEnd(boundaryPointInPlane(data->secPoint));
                    builder.setLayer(layer);
                    loopData.push_back(builder.build());
                } else if(k->eType == DRW::ETYPE::ARC) { //done
                    auto data = std::dynamic_pointer_cast<DRW_Arc>(k);
                    // Same geo::Arc precondition as addArc: a hatch boundary may
                    // carry a zero-radius arc edge, which used to abort the open.
                    if (!(data->radious > 0.0)) {
                        LOG_ERROR << "Skipping HATCH boundary ARC with unusable radius " << data->radious;
                        continue;
                    }
                    lc::builder::ArcBuilder builder;
                    LOG_TRACE << data->staangle <<','<< data->endangle;
                    builder.setCenter(boundaryPointInPlane(data->basePoint));
                    builder.setRadius(data->radious);
                    builder.setStartAngle(data->staangle);
                    builder.setEndAngle(data->endangle);

                    builder.setIsCCW((bool) data->isccw);
                    builder.setLayer(layer);
                    loopData.push_back(builder.build());
                } else if(k->eType == DRW::ETYPE::ELLIPSE) { //done
                    auto data = std::dynamic_pointer_cast<DRW_Ellipse>(k);
                    auto secPoint = coord(data->secPoint);
                    auto lcEllipse = std::make_shared<lc::entity::Ellipse>(boundaryPointInPlane(data->basePoint),
                                     secPoint,
                                     secPoint.magnitude() * data->ratio,
                                     data->staparam,
                                     data->endparam,
                                     !data->isccw,
                                     layer
                                                                          );
                    loopData.push_back(lcEllipse);
                } else if(k->eType == DRW::ETYPE::SPLINE) {
                    auto data = std::dynamic_pointer_cast<DRW_Spline>(k);
                    // Same as addSpline: keep the file's knot vector intact.
                    auto lcSpline = std::make_shared<lc::entity::Spline>(pointsInOffsetPlane(data->controllist),
                                    data->knotslist,
                                    pointsInOffsetPlane(data->fitlist),
                                    data->degree,
                                    false,
                                    data->tolfit,
                                    data->tgStart.x, data->tgStart.y, data->tgStart.z,
                                    data->tgEnd.x, data->tgEnd.y, data->tgEnd.z,
                                    data->normalVec.x, data->normalVec.y, data->normalVec.z,
                                    static_cast<lc::geo::Spline::splineflag>(data->flags),
                                    layer,
                                    mf,
                                    getBlock(*data), data->weightlist
                                                                        );
                    noteRationalSplineApproximation(_loss, *data);
                    loopData.push_back(lcSpline);
                }
            }
            lc::geo::Loop loop(loopData);
            reg.addLoop(loop);
        }
        lcHatch->setRegion(reg);
        deliver(lcHatch);
        // The default libdxfrw callback forwards MPOLYGON to addHatch. The
        // kernel currently has no way to retain its subtype or extra fields.
        if (data->eType == DRW::MPOLYGON && _mpolygonsAsHatch++ == 0) {
            _loss.notes.push_back(
                "MPOLYGON entities were imported as HATCH for editing. Some "
                "polygon-specific information may not survive saving.");
        }
        if (polygon && ((!liftBoundaryPlane && polygon->basePoint.z != 0.0) || polygon->extPoint.x != 0.0
                        || polygon->extPoint.y != 0.0 || polygon->extPoint.z != 1.0)
            && _mpolygonsWithNondefaultPlane++ == 0) {
            _loss.notes.push_back(
                "MPOLYGON elevation or extrusion is not fully mapped into "
                "the editing plane; its displayed geometry may be incorrect.");
        }
    });
}

lc::meta::Block_CSPtr DXFimpl::getBlock(const DRW_Entity& data) const {
    lc::meta::Block_CSPtr block;
    auto x = _handleBlock.find(data.parentHandle);
    if(x!=_handleBlock.end()) {
        block = data.parentHandle?x->second:_currentBlock;
    } else {
        block = _currentBlock;
    }
    if(block) {
        // DXF symbol-table names are case-insensitive, and writers disagree:
        // AutoCAD writes *Model_Space, ODA's converter writes *MODEL_SPACE for
        // R13 and R14. An exact comparison treated those files' model space as
        // an ordinary named block, so every model-space entity landed inside a
        // block and the drawing opened empty.
        if(isModelSpaceName(block->name())) {
            return nullptr;
        }

        // Paper space is deliberately left alone. LibreCAD 3 has no layout
        // concept, so *Paper_Space stays an ordinary block: its entities are
        // preserved and reachable, but they do not join the model-space
        // drawing, where they would overlay it at layout coordinates. When
        // layouts arrive, this is the place that decides.
    }
    return block;
}

bool DXFimpl::isModelSpaceName(const std::string& name) {
    static const std::string modelSpace = DEFAULT_VIEWPORT;
    return name.size() == modelSpace.size()
           && std::equal(name.begin(), name.end(), modelSpace.begin(),
                         [](char a, char b) {
                             return std::tolower(static_cast<unsigned char>(a))
                                    == std::tolower(static_cast<unsigned char>(b));
                         });
}

lc::meta::Layer_CSPtr DXFimpl::getLayer(const DRW_Entity& data) {
    const auto pending = _layers.find(upperName(data.layer));
    if (pending != _layers.end()) return pending->second;
    lc::meta::Layer_CSPtr layer = _document->layerByName(data.layer);

    if (layer==nullptr) {
        auto col = icol.intToColor(255);
        auto lw = getLcLineWidth<lc::meta::MetaLineWidthByValue>(DRW_LW_Conv::lineWidth::width00);
        auto lp = _document->linePatternByName("CONTINUOUS");
        auto isFrozen = false;
        // we need it anyway so,
        layer = std::make_shared<lc::meta::Layer>(data.layer, lw->width(), col->color(), lp, isFrozen);
        auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
        _builder->append(al);
        _layers[upperName(data.layer)] = layer;
    }
    return layer;
}

lc::meta::MetaInfo_SPtr DXFimpl::getMetaInfo(const DRW_Entity& data) const {
    std::shared_ptr<lc::meta::MetaInfo> mf = nullptr;

    // Try to find a entities meta line weight
    auto lw = getLcLineWidth<lc::meta::MetaLineWidth>(data.lWeight);
    if (lw != nullptr) {
        if (mf == nullptr) {
            mf = lc::meta::MetaInfo::create();
        }

        mf->add(lw);
    }

    // Try to find a entities meta color
    lc::meta::MetaColor_CSPtr col;
    if (data.color24 >= 0 && data.color24 <= 0xFFFFFF) {
        const int rgb = data.color24;
        col = std::make_shared<const lc::meta::MetaColorTrueColor>(
            lc::Color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF),
            data.color, data.colorName);
    } else if(data.color == BYBLOCK_COLOR) {
        col = std::make_shared<const lc::meta::MetaColorByBlock>();
    }
    else {
        col = icol.intToColor(data.color);
    }

    if (col != nullptr) {
        if (mf == nullptr) {
            mf = lc::meta::MetaInfo::create();
        }

        mf->add(col);
    }

    // Most likely a lot of entities within a drawing will be 'BYLAYER' and with the CONTINUOUS linetype.
    // These are the default's for LibreCAD
    lc::meta::DxfLinePattern_CSPtr linePattern = nullptr;
    if(data.lineType == LTYPE_BYBLOCK) {
        linePattern = std::make_shared<lc::meta::DxfLinePatternByBlock>();
    }
    else if (!(lc::tools::StringHelper::cmpCaseInsensitive()(data.lineType, SKIP_BYLAYER) || lc::tools::StringHelper::cmpCaseInsensitive()(data.lineType, SKIP_CONTINUOUS))) {
        linePattern = _document->linePatternByName(data.lineType);
    }

    if(linePattern != nullptr) {
        if (mf == nullptr) {
            mf = lc::meta::MetaInfo::create();
        }

        mf->add(linePattern);
    }

    return mf;
}

lc::geo::Coordinate DXFimpl::coord(DRW_Coord const& coord) const {
    return { coord.x, coord.y, coord.z };
}

std::vector<lc::geo::Coordinate> DXFimpl::coords(std::vector<std::shared_ptr<DRW_Coord>> coordList) const {
    std::vector<lc::geo::Coordinate> coords;
    coords.reserve(coordList.size());
    for (const auto& ptr : coordList) {
        coords.emplace_back(ptr->x,ptr->y, ptr->z);
    }
    return coords;
}

void DXFimpl::addLType(const DRW_LType& data) {
    guarded("LTYPE", nullptr, [&] {
        std::make_shared<lc::operation::AddLinePattern>(_document, std::make_shared<lc::meta::DxfLinePatternByValue>(data.name, data.desc, data.path, data.length))->execute();
    });
}

/**
 * I am not sure the order of addImage and linkImage, if they can be swapped around or not
 * so if we see missing images, it could be that we first get calls to linImage and then to addImage
 * if linkImage isn't called as last, we miss a image during import
 */
void DXFimpl::addImage(const DRW_Image* data) {
    guarded("IMAGE", data, [&] {
        LOG_TRACE << "addImage";
        imageMapCache.emplace_back(*data);
    });
}

#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
void DXFimpl::addAppId(const DRW_AppId& data) {
    guarded("APPID", nullptr, [&] {
        if (data.name.empty() || !simpleImageAscii(data.name, 31)) return;
        const auto key = upperName(data.name);
        const bool simple = data.flags == 0 && data.unknown71 == 0 && data.extData.empty() &&
            data.appData.empty() && data.reactorHandles.empty() && data.xDictHandle == 0 &&
            !data.hasDataStorageBinaryData() && data.pendingAppIdResolutions.empty() &&
            data.pendingLayerRefResolutions.empty();
        const auto found = _sourceImageAppIds.find(key);
        if (found != _sourceImageAppIds.end()) { found->second = false; return; }
        if (_sourceImageAppIds.size() < 4096) _sourceImageAppIds.emplace(key, simple);
    });
}
#endif

void DXFimpl::linkImage(const DRW_ImageDef *data) {
    guarded("IMAGEDEF", nullptr, [&] {
        LOG_TRACE << "linkImage";
        std::shared_ptr<ImageDefinitionSource> definition;
        for(auto image = imageMapCache.begin(); image != imageMapCache.end() /* not hoisted */; /* no increment */ ) {
            if (image->ref == data->handle) {
                if (data->handle != 0 && data->handle != DRW::NoHandle)
                    _preserved.sourceImageDefinitionHandles.insert(data->handle);
                if (image->m_imageDefReactorHandle != 0 && image->m_imageDefReactorHandle != DRW::NoHandle)
                    _preserved.sourceImageReactorHandles.insert(image->m_imageDefReactorHandle);
                auto layer = _document->layerByName(image->layer);

                std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*image);
                if (!mf) mf = lc::meta::MetaInfo::create();
                if (!definition) {
                    definition = std::make_shared<ImageDefinitionSource>();
                    constexpr std::size_t kMaxImageDefinitionPathBytes = 4u * 1024u * 1024u;
                    if (data->name.size() <= kMaxImageDefinitionPathBytes - _sourceImageDefinitionPathBytes) {
                        definition->path = data->name;
                        definition->pathCaptured = true;
                        _sourceImageDefinitionPathBytes += data->name.size();
                    }
                    definition->version = data->imgVersion;
                    definition->u = data->u; definition->v = data->v;
                    definition->up = data->up; definition->vp = data->vp;
                    definition->loaded = data->loaded; definition->resolution = data->resolution;
                    bool appDataRetained = data->appData.empty();
#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
                    if (!appDataRetained)
                        appDataRetained = captureSimpleImageDefinitionAppData(data->appData, definition->appData,
                                                                              _sourceImageDefinitionMetadataBytes);
#endif
                    bool xdataRetained = data->extData.empty();
#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
                    if (!xdataRetained && data->pendingAppIdResolutions.empty() && data->pendingLayerRefResolutions.empty())
                        xdataRetained = captureSimpleImageDefinitionXdata(data->extData, definition->xdata,
                                                                          _sourceImageDefinitionMetadataBytes);
                    definition->xdataLayers.resize(definition->xdata.size());
                    for (std::size_t i = 0; i < definition->xdata.size(); ++i) {
                        const auto& value = definition->xdata[i];
                        if (value.code() != 1003 || !value.content.s) continue;
                        const auto pending = _layers.find(upperName(*value.content.s));
                        definition->xdataLayers[i] = pending != _layers.end() ? pending->second :
                            _document->layerByName(*value.content.s);
                    }
                    for (const auto& value : definition->xdata) {
                        if (value.code() != 1001 || !value.content.s) continue;
                        const auto appId = _sourceImageAppIds.find(upperName(*value.content.s));
                        if (appId == _sourceImageAppIds.end() || !appId->second) xdataRetained = false;
                    }
#endif
                    definition->unmodeledMetadata = !xdataRetained || !appDataRetained ||
                        !data->reactorHandles.empty() || !data->reactors.empty() || data->xDictHandle != 0 ||
                        data->hasDataStorageBinaryData() || !data->pendingAppIdResolutions.empty() ||
                        !data->pendingLayerRefResolutions.empty();
                }
                auto source = std::make_shared<ImageDisplaySource>();
                source->classVersion = image->m_classVersion;
                source->displayProps = image->m_displayProps;
                source->clip = image->clip;
                source->boundaryType = image->m_clipBoundaryType;
                source->clipMode = image->clipMode;
                source->definitionHandle = image->ref;
                source->sourceIdentity = _preserved.imageSourceIdentity;
                source->definition = definition;
                source->width = image->sizeu;
                source->height = image->sizev;
                constexpr std::size_t kMaxImageClipBytes = 4u * 1024u * 1024u;
                if (image->clipPath.size() > (kMaxImageClipBytes - _sourceImageClipBytes) / sizeof(DRW_Coord)) {
                    source->boundaryDropped = true;
                } else {
                    _sourceImageClipBytes += image->clipPath.size() * sizeof(DRW_Coord);
                    source->boundary = std::move(image->clipPath);
                }
                mf->add(source);
                const lc::geo::Coordinate base(coord(image->basePoint));
                const lc::geo::Coordinate uv(coord(image->secPoint));
                const lc::geo::Coordinate vv(coord(image->vVector));

                auto lcImage = std::make_shared<lc::entity::Image>(
                                   data->name,
                                   base, uv, vv,
                                   image->sizeu, image->sizev,
                                   image->brightness, image->contrast, image->fade,
                                   layer,
                                   mf,
                                   getBlock(*image)
                               );
                deliver(lcImage);

                image = imageMapCache.erase( image ) ; // advances iter
            } else {
                image++;
            }
        }
    });
}

void DXFimpl::addInsert(const DRW_Insert& data) {
    guarded("INSERT", &data, [&] {
        LOG_TRACE << "addInsert " << data.name;

        // Recorded, not built: see buildDeferredInserts().  Building here measured
        // an empty block for the bounding box, and -- because the referenced block
        // is not in the document during the read -- invented a second Block object
        // with the same name for the entity to point at, which then never matched
        // the real one by identity.
        _pendingInserts.push_back(PendingInsert{
            getMetaInfo(data),
            getBlock(data),
            getLayer(data),
            coord(data.basePoint),
            data.name});

        // ATTRIBs are the values filled into a block's placeholders -- a title
        // block's drawing number, a door's width -- and were dropped in
        // silence, so those drawings came back with their labels missing and
        // looked merely wrong rather than incomplete.
        //
        // LibreCAD has no attribute model: nothing ties a value to the INSERT
        // it belongs to, or remembers its tag. What it does have is text at a
        // position, which is what an ATTRIB draws, so visible values arrive as
        // Text or MText. The tag does not survive, and a later save writes the
        // text value rather than ATTRIB -- the note records that limitation.
        for (const auto& attrib : data.attlist) {
            if (attrib == nullptr) {
                continue;
            }

            // Bit 0 = invisible: AutoCAD does not draw these, so neither do we.
            if ((attrib->attribFlags & 1u) != 0u) {
                recordLoss("ATTRIB (invisible)");
                continue;
            }

            if (attrib->mtext) {
                deliver(makeMText(*attrib->mtext, *attrib));
            }
            else {
                deliver(std::make_shared<lc::entity::Text>(
                    coord(attrib->basePoint),
                    attrib->text, attrib->height,
                    attrib->angle * M_PI / 180, attrib->style,
                    lc::TextConst::DrawingDirection(attrib->textgen),
                    lc::TextConst::HAlign(attrib->alignH),
                    lc::TextConst::VAlign(attrib->alignV),
                    getLayer(*attrib),
                    getMetaInfo(*attrib),
                    getBlock(data)));
            }

            if (_attributesWithoutTags++ == 0) {
                _loss.notes.push_back(
                    "Block attributes were imported without their tags: LibreCAD has "
                    "no attribute model, so tags are not kept and a save writes their "
                    "values as TEXT or MTEXT.");
            }
        }
    });
}

void DXFimpl::buildDeferredInserts() {
    if (_pendingInserts.empty()) {
        return;
    }

    std::vector<bool> built(_pendingInserts.size(), false);
    // A block referenced but never defined -- an xref, or a truncated file --
    // still gets an INSERT, pointing at an empty block of that name.  One per
    // name: fabricating one per INSERT is how duplicate same-named blocks got
    // into the document in the first place.
    std::map<std::string, lc::meta::Block_CSPtr> fabricated;
    size_t remaining = _pendingInserts.size();

    while (remaining > 0) {
        // An INSERT of block T can only be measured once every INSERT *inside*
        // T exists, or a nested block contributes nothing to the box.  Blocks
        // nest arbitrarily deep, so this walks the dependency graph one layer
        // at a time rather than assuming a depth.
        std::vector<size_t> round;
        for (size_t i = 0; i < _pendingInserts.size(); i++) {
            if (built[i]) {
                continue;
            }

            const auto& target = _pendingInserts[i].targetBlockName;
            bool waiting = false;
            for (size_t j = 0; j < _pendingInserts.size() && !waiting; j++) {
                waiting = !built[j] && j != i
                          && _pendingInserts[j].containerBlock != nullptr
                          && _pendingInserts[j].containerBlock->name() == target;
            }

            if (!waiting) {
                round.push_back(i);
            }
        }

        if (round.empty()) {
            // A block that inserts itself, directly or through a cycle. The
            // file is malformed; build what is left in file order so the
            // entities still appear, and accept the box a cycle allows.
            LOG_WARNING << "Recursive block reference; building the remaining "
                        << remaining << " INSERT(s) unordered";
            for (size_t i = 0; i < _pendingInserts.size(); i++) {
                if (!built[i]) {
                    round.push_back(i);
                }
            }
        }

        auto builder = std::make_shared<lc::operation::Builder>(_document, "Insert blocks");
        auto entityBuilder = std::make_shared<lc::operation::EntityBuilder>(_document);

        for (const auto i : round) {
            const auto& pending = _pendingInserts[i];
            built[i] = true;
            remaining--;

            auto block = _document->blockByName(pending.targetBlockName);
            if (block == nullptr) {
                auto known = fabricated.find(pending.targetBlockName);
                if (known != fabricated.end()) {
                    block = known->second;
                } else {
                    LOG_WARNING << "INSERT references block " << pending.targetBlockName
                                << ", which this file does not define; inserting it empty";
                    block = std::make_shared<lc::meta::Block>(pending.targetBlockName,
                                                             geo::Coordinate());
                    fabricated[pending.targetBlockName] = block;
                    builder->append(std::make_shared<lc::operation::AddBlock>(_document, block));
                }
            }

            // Guarded for the same reason the callbacks are: this runs after
            // the read, outside libdxfrw, so a throw here would leave
            // File::open by a path nothing catches.
            guarded("INSERT", nullptr, [&] {
                lc::builder::InsertBuilder insertBuilder;
                insertBuilder.setMetaInfo(pending.metaInfo);
                insertBuilder.setBlock(pending.containerBlock);
                insertBuilder.setLayer(pending.layer);
                insertBuilder.setCoordinate(pending.position);
                insertBuilder.setDisplayBlock(block);
                insertBuilder.setDocument(_document);

                _entitiesDelivered++;
                entityBuilder->appendEntity(insertBuilder.build());
            });
        }

        builder->append(entityBuilder);
        builder->execute();
    }

    _pendingInserts.clear();
}

/*********************************************
 * Write DXF Implementation BELOW
 *********************************************/

void DXFimpl::visitDocumentImages(const std::function<void(const lc::entity::Image_CSPtr&)>& visit,
                                  bool emittedOnly) const {
#ifndef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    (void)emittedOnly;
#endif
    const auto collect = [&](const lc::entity::CADEntity_CSPtr& entity) {
        const auto image = std::dynamic_pointer_cast<const lc::entity::Image>(entity);
        if (image) visit(image);
    };
    for (const auto& entity : _document->entityContainer().asVector())
        if (!entity->block()) collect(entity);
    for (const auto& block : _document->blocks()) {
        if (!block || isRegeneratedDimensionBlock(block)) continue;
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        if (emittedOnly && !_defaultLayouts.empty() && isModelSpaceName(block->name())) continue;
#endif
        for (const auto& entity : _document->entitiesByBlock(block).asVector()) collect(entity);
    }
}

std::unordered_set<std::uint32_t> DXFimpl::skippedImageSourceHandles(bool& hasImages, const PreservedRecords* source) const {
    hasImages = false;
    std::unordered_set<std::uint32_t> skipped;
    visitDocumentImages([&](const lc::entity::Image_CSPtr& image) {
        hasImages = true;
        const auto imageSource = image->metaInfo<ImageDisplaySource>(ImageDisplaySource::key());
        if (source && imageSource && imageSource->sourceIdentity == source->imageSourceIdentity &&
            imageSource->definitionHandle != 0 && imageSource->definitionHandle != DRW::NoHandle)
            skipped.insert(imageSource->definitionHandle);
    });
    if (!source) return skipped;
    skipped.insert(source->sourceImageDefinitionHandles.begin(), source->sourceImageDefinitionHandles.end());
    for (const auto& object : source->objects)
        if (skipsSourceImageObject(object, hasImages, source) && object.handle != 0)
            skipped.insert(object.handle);
    for (const auto& object : source->objects)
        if (!skipsSourceImageObject(object, hasImages, source)) skipped.erase(object.handle);
    for (const auto& entity : source->entities) skipped.erase(entity.handle);
    return skipped;
}

#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
bool DXFimpl::prepareImageDefinitionPlan() {
    using Key = std::pair<std::uint64_t, std::uint32_t>;
    std::map<Key, dxfRW::ImageDefinitionPlanEntry> definitions;
    std::map<Key, std::shared_ptr<const ImageDefinitionSource>> sources;
    bool ambiguous = false;
    visitDocumentImages([&](const lc::entity::Image_CSPtr& image) {
        const auto source = image->metaInfo<ImageDisplaySource>(ImageDisplaySource::key());
        if (!source || !source->sourceIdentity || !source->definition ||
            source->definitionHandle == 0 || source->definitionHandle == DRW::NoHandle ||
            !validImageDefinitionSource(*source->definition, image->name())) return;
        auto generation = _imageSourceGenerations.find(source->sourceIdentity);
        if (generation == _imageSourceGenerations.end()) {
            if (_imageSourceGenerations.size() >= 100000u) { ambiguous = true; return; }
            const auto next = static_cast<std::uint64_t>(_imageSourceGenerations.size()) + 1;
            generation = _imageSourceGenerations.emplace(source->sourceIdentity, next).first;
        }
        const Key key{generation->second, source->definitionHandle};
        const auto& original = *source->definition;
        const auto existing = definitions.find(key);
        if (existing != definitions.end()) {
            if (sources.at(key) != source->definition) ambiguous = true;
            return;
        }
        if (definitions.size() >= 100000u) { ambiguous = true; return; }
        dxfRW::ImageDefinitionPlanEntry entry;
        entry.importGeneration = generation->second;
        entry.sourceHandle = source->definitionHandle;
        entry.definition.handle = source->definitionHandle;
        entry.definition.name = original.path;
        entry.definition.imgVersion = original.version;
        entry.definition.u = original.u; entry.definition.v = original.v;
        entry.definition.up = original.up; entry.definition.vp = original.vp;
        entry.definition.loaded = original.loaded; entry.definition.resolution = original.resolution;
        entry.definition.appData = original.appData;
        sources.emplace(key, source->definition);
        definitions.emplace(key, std::move(entry));
    }, true);
    if (ambiguous) return false;
    for (const auto& item : definitions) _plannedImageDefinitions.insert(item.first);
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
    std::uint64_t boundGeneration = 0;
    if (_shelf && _replay == _shelf && !_shelf->sourceWasDwg && !_shelf->rootDictSourceAmbiguous &&
        _shelf->simpleImageRoot && _shelf->simpleImageDictionary &&
        _shelf->simpleImageRootVersion == _exportVersion && _shelf->version == _exportVersion &&
        _shelf->entities.empty() && _shelf->sections.empty() && _shelf->classes.empty() &&
        !_shelf->hasSourceLayouts && _imageSourceGenerations.size() == 1) {
        const auto generation = _imageSourceGenerations.find(_shelf->imageSourceIdentity);
        const auto& dict = *_shelf->simpleImageDictionary;
        const DRW_RawDxfObject* physical = nullptr;
        bool safe = generation != _imageSourceGenerations.end() && dict.m_entries.size() == definitions.size();
        for (const auto& entry : dict.m_entries)
            if (!safe || !_plannedImageDefinitions.count({generation->second, entry.m_handle})) safe = false;
        for (const auto& object : _shelf->objects) {
            if (object.name == "DICTIONARY" && object.handle == dict.handle && !physical) physical = &object;
            else if (object.name != "IMAGEDEF_REACTOR" || !_shelf->sourceImageReactorHandles.count(object.handle)) safe = false;
        }
        if (safe && physical && physical->m_version == _exportVersion &&
            dict.parentHandle == _shelf->simpleImageRoot->handle && simpleImageDictionaryMatches(dict, *physical)) {
            _boundSourceImageDictionary = dict.handle;
            boundGeneration = generation->second;
        }
    }
#endif
    std::vector<dxfRW::ImageDefinitionPlanEntry> plan;
    plan.reserve(definitions.size());
    for (auto& item : definitions) {
        const auto& key = item.first;
        auto& entry = item.second;
        const auto& original = *sources.at(key);
        const auto sourceGeneration = _shelf ? _imageSourceGenerations.find(_shelf->imageSourceIdentity) : _imageSourceGenerations.end();
        for (std::size_t begin = 0; begin < original.xdata.size();) {
            std::size_t end = begin + 1;
            while (end < original.xdata.size() && original.xdata[end].code() != 1001) ++end;
            bool valid = original.xdataLayers.size() == original.xdata.size();
            for (std::size_t i = begin; valid && i < end; ++i) {
                const auto& value = original.xdata[i];
                if (value.code() == 1003) {
                    const auto captured = original.xdataLayers[i].lock();
                    const auto current = value.content.s ? _document->layerByName(*value.content.s) : nullptr;
                    valid = sourceGeneration != _imageSourceGenerations.end() && sourceGeneration->second == key.first &&
                        captured && current && current->retainsColorFrom(captured) &&
                        upperName(current->name()) == upperName(*value.content.s);
                }
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
                if (value.code() == 1005) {
                    std::uint32_t target = 0;
                    valid = boundGeneration != 0 && key.first == boundGeneration && sourceHandle(value, target) &&
                        _plannedImageDefinitions.count({boundGeneration, target}) != 0;
                }
#endif
            }
            if (!valid) _imageXdataPartialSources.insert(key);
            else {
                for (std::size_t i = begin; i < end; ++i) {
                    const auto& value = original.xdata[i];
                    if (!entry.definition.addExtData(std::make_unique<DRW_Variant>(value))) return false;
                    if (value.code() == 1001 && value.content.s)
                        _imageXdataAppIds.emplace(upperName(*value.content.s), *value.content.s);
                }
            }
            begin = end;
        }
        plan.push_back(std::move(entry));
    }
    if (!dxfW->setImageDefinitionPlan(std::move(plan))) return false;
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
    if (boundGeneration != 0) {
        if (!dxfW->setGeneratedDictionarySourceRecords(
                std::make_unique<DRW_Dictionary>(*_shelf->simpleImageRoot), nullptr,
                std::make_unique<DRW_Dictionary>(*_shelf->simpleImageDictionary), boundGeneration)) return false;
        const auto loss = _loss.droppedByType.find("unreferenced dictionaries");
        if (loss != _loss.droppedByType.end() && loss->second > 0 && --loss->second == 0)
            _loss.droppedByType.erase(loss);
    }
#endif
    return true;
}
#endif

void DXFimpl::writeObjects() {
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    if (!_defaultLayoutsFromSource)
        for (auto& layout : _defaultLayouts)
            _defaultLayoutWriteOk = dxfW->writeLayout(&layout) && _defaultLayoutWriteOk;
#endif
    if (_replay == nullptr) {
        return;
    }

    // An IMAGE in the document makes the typed writer emit its own IMAGEDEF and
    // IMAGEDEF_REACTOR. Replaying the source's as well leaves the file with two
    // of each for one image, so the preserved copies give way to the regenerated
    // ones -- measured: without this, sample_AC1015_ascii comes back with two
    // reactors.
    bool documentHasImages = false;
    const auto skippedImageHandles = skippedImageSourceHandles(documentHasImages, _replay.get());

    for (const auto& object : _replay->objects) {
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
        if (_boundSourceImageDictionary != 0 && object.name == "DICTIONARY" &&
            object.handle == _boundSourceImageDictionary) continue;
#endif
        if (skipsSourceImageObject(object, documentHasImages, _replay.get())) {
            // The regenerated record has not been shown to retain the source
            // record's metadata or identity.
            ++_loss.droppedByType[object.name];
            continue;
        }

        DRW_RawDxfObject copy = object;
        const auto droppedEntries = dropRegeneratedImageEntries(copy, skippedImageHandles);
        if (!dxfW->writeRawDxfObject(&copy)) {
            LOG_ERROR << "libdxfrw refused to re-emit a preserved " << object.name << " record";
            return;
        }
        if (droppedEntries) _loss.droppedByType["dictionary entries"] += droppedEntries;
    }
}

void DXFimpl::writeLayers() {
    auto layers = _document->allLayers();
    for(const auto& layer: layers) {
        writeLayer(layer.second);
    }
}

void DXFimpl::writeLayer(const std::shared_ptr<const lc::meta::Layer>& layer) {
    DRW_Layer lay;
    lc::iColor icol_inst;

    auto col = layer->color();
    lay.name = layer->name();
    lay.color = icol_inst.colorToInt(col, 1);
    const PreservedRecords::SourceLayerColor* original = nullptr;
    if (_shelf) {
        const auto found = _shelf->sourceLayerColors.find(layer->name());
        if (found != _shelf->sourceLayerColors.end()) original = &found->second;
    }
    const bool unchanged = original && layer->retainsColorFrom(original->layer.lock());
    const bool explicitRgb = unchanged && original->rgb >= 0 && original->rgb <= 0xFFFFFF;
    const auto indexed = icol_inst.intToColor(lay.color);
    const bool needsRgb = explicitRgb || !indexed || !(indexed->color() == col);
    if (unchanged && (!explicitRgb || _exportVersion > DRW::AC1015) &&
        original->aci != 0 && original->aci >= -255 && original->aci <= 255)
        lay.color = original->aci;
    if (lay.color < 0) lay.color = -lay.color;
    if (layer->isOff()) lay.color = -lay.color;
    if (_exportVersion > DRW::AC1015) {
        if (needsRgb)
            lay.color24 = (static_cast<int>(col.redI()) << 16)
                        | (static_cast<int>(col.greenI()) << 8) | static_cast<int>(col.blueI());
        if (unchanged) lay.colorName = original->name;
    } else if (needsRgb || (unchanged && !original->name.empty())) {
        const std::string note = "Layer true colors or color names were approximated using indexed colors for the selected DXF version.";
        if (std::find(_loss.notes.begin(), _loss.notes.end(), note) == _loss.notes.end())
            _loss.notes.push_back(note);
    }
    if (original && !unchanged && !original->name.empty()) {
        const std::string note = "Editing a layer may discard its source color name.";
        if (std::find(_loss.notes.begin(), _loss.notes.end(), note) == _loss.notes.end())
            _loss.notes.push_back(note);
    }

    // Issue #412 phase 3: (a) stop the stray `std::cout << wid;` that has
    // been dumping the layer line width to stdout on every save since 2016.
    // (b) actually emit the layer line weight — the conversion was commented
    // out for a decade, so every saved layer defaulted to widthDefault.
    auto wid = layer->lineWidth().width();
    int lwIdx = widthToInt(wid);
    if (lwIdx >= 0) {
        lay.lWeight = static_cast<DRW_LW_Conv::lineWidth>(lwIdx);
    } else {
        lay.lWeight = DRW_LW_Conv::widthDefault;
    }
    if (_exportVersion <= DRW::AC1014 && lay.lWeight != DRW_LW_Conv::widthDefault) {
        bool implicitDefault = false;
        if (_shelf) {
            const auto source = _shelf->sourceLayerWeights.find(layer->name());
            if (source != _shelf->sourceLayerWeights.end()) {
                implicitDefault = source->second.implicitDefault && source->second.kernelWidth == wid;
            } else {
                // DocumentImpl creates layer 0 at this width before importing
                // any layer. A source layer, when present, takes precedence.
                implicitDefault = layer->name() == "0" && wid == 1.0;
            }
        } else {
            implicitDefault = layer->name() == "0" && wid == 1.0;
        }
        if (!implicitDefault) _loss.droppedByType["layer lineweight"]++;
        lay.lWeight = DRW_LW_Conv::widthDefault;
    }

    // Issue #412 phase 3: emit the layer's line pattern name (code 6) so the
    // reader can match entities that reference it back to the layer LTYPE.
    // Skip an empty/unset pattern; libdxfrw defaults to "CONTINUOUS".
    auto lp = layer->linePattern();
    if (lp != nullptr && !lp->name().empty()) {
        lay.lineType = lp->name();
    }

    lay.flags = (layer->isFrozen() ? 0x01 : 0x00) | (layer->isLocked() ? 0x04 : 0x00);

    dxfW->writeLayer(&lay);
}

/// The DXF revision a writable DXF/DXB target names, or UNKNOWNV for a target
/// that is not one -- the same mapping writeDXF() applies, kept next to the one
/// other place that needs it.
static DRW::Version dxfRevisionForType(lc::persistence::File::Type type) {
    switch (type) {
    case lc::persistence::File::LIBDXFRW_DXF_R12:
    case lc::persistence::File::LIBDXFRW_DXB_R12:
        return DRW::AC1009;
    case lc::persistence::File::LIBDXFRW_DXF_R14:
    case lc::persistence::File::LIBDXFRW_DXB_R14:
        return DRW::AC1014;
    case lc::persistence::File::LIBDXFRW_DXF_R2000:
    case lc::persistence::File::LIBDXFRW_DXB_R2000:
        return DRW::AC1015;
    case lc::persistence::File::LIBDXFRW_DXF_R2004:
    case lc::persistence::File::LIBDXFRW_DXB_R2004:
        return DRW::AC1018;
    case lc::persistence::File::LIBDXFRW_DXF_R2007:
    case lc::persistence::File::LIBDXFRW_DXB_R2007:
        return DRW::AC1021;
    case lc::persistence::File::LIBDXFRW_DXF_R2010:
    case lc::persistence::File::LIBDXFRW_DXB_R2010:
        return DRW::AC1024;
    case lc::persistence::File::LIBDXFRW_DXF_R2013:
    case lc::persistence::File::LIBDXFRW_DXB_R2013:
        return DRW::AC1027;
    case lc::persistence::File::LIBOPENCAD_DWG:
    case lc::persistence::File::LIBDXFRW_DWG_IMPORT:
        return DRW::UNKNOWNV;
    }
    return DRW::UNKNOWNV;
}

namespace {

/// The handle libdxfrw's DXF codec always regenerates the root
/// NamedObjectsDictionary at.
constexpr std::uint32_t kRootDictionaryHandle = 0xC;

/// And the ACAD_GROUP dictionary beside it, which the codec also regenerates
/// and fills from setGroups().
constexpr std::uint32_t kGroupDictionaryHandle = 0xD;

}  // namespace

#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
void DXFimpl::addDxfRootDictionarySource(const DRW_Dictionary& data, const DRW_RawDxfObject& raw) {
    guarded("ROOT IMAGE DICTIONARY", nullptr, [&] {
        if (data.handle != kRootDictionaryHandle || data.parentHandle != 0 ||
            data.m_entries.size() != 1 || data.m_entries.front().m_name != "ACAD_IMAGE_DICT" ||
            data.m_entries.front().m_handle == kRootDictionaryHandle ||
            data.m_entries.front().m_handle == kGroupDictionaryHandle ||
            !simpleImageDictionaryMatches(data, raw)) return;
        _preserved.simpleImageRoot = std::make_shared<DRW_Dictionary>(data);
        _preserved.simpleImageRootVersion = raw.m_version;
    });
}
#endif

void DXFimpl::addDictionary(const DRW_Dictionary& data) {
    guarded("DICTIONARY", nullptr, [&] {
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
        if (_preserved.simpleImageRoot &&
            data.handle == _preserved.simpleImageRoot->m_entries.front().m_handle) {
            if (_preserved.simpleImageDictionary || _ambiguousSimpleImageDictionary) {
                _preserved.simpleImageDictionary.reset();
                _ambiguousSimpleImageDictionary = true;
            } else if (simpleImageDictionaryTyped(data)) {
                _preserved.simpleImageDictionary = std::make_shared<DRW_Dictionary>(data);
            } else _ambiguousSimpleImageDictionary = true;
        }
#endif
        // libdxfrw calls this for every DICTIONARY, root or not, and separately
        // routes the non-root ones into the raw net. Only the root's entries
        // are wanted here: every other dictionary is re-emitted verbatim,
        // carrying its own entries with it, and what is missing is the entry in
        // the root that named it.
        //
        // So the test has to be the same test libdxfrw uses, or the two
        // disagree about one dictionary and its children get named twice --
        // once by the preserved dictionary and once, directly, from the
        // regenerated root. Asking only whether an owner group was present is
        // what libdxfrw used to do and stopped doing, because code 330 is
        // optional on a DICTIONARY: an owner-less dictionary that is not the
        // root is ordinary, and 284 of 1840 real drawings hold one.
        const bool firstInObjects = !_seenDictionary;
        _seenDictionary = true;

        // ACAD_GROUP is regenerated too, and its entries name groups rather
        // than children of the root.
        if (data.handle == kGroupDictionaryHandle) {
            return;
        }

        const bool root = data.handle == kRootDictionaryHandle
                          || (firstInObjects && data.parentHandle == 0);
        if (!root) {
            return;
        }
        if (_preserved.rootDictSourceHandle != 0)
            _preserved.rootDictSourceAmbiguous = true;
        _preserved.rootDictSourceHandle = data.handle;
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        _sourceRootHandle = data.handle;
#endif

        for (const auto& entry : data.m_entries) {
            if (entry.m_handle == 0 || entry.m_name.empty()) {
                continue;
            }
            if (upperName(entry.m_name) == "ACAD_GROUP") {
                if (_preserved.groupDictSourceHandle != 0) _preserved.rootDictSourceAmbiguous = true;
                _preserved.groupDictSourceHandle = entry.m_handle;
            }
            // The codec writes its own entries for the dictionaries it
            // regenerates. Carrying the source's copy across would put a
            // second ACAD_GROUP key in the root -- and, since the codec does
            // not route handle D into the raw net, it could never be spliced
            // anyway, so it would be counted as a loss on every ordinary
            // drawing.
            if (entry.m_handle == kRootDictionaryHandle
                || entry.m_handle == kGroupDictionaryHandle) {
                continue;
            }
            _preserved.rootDictEntries.emplace_back(entry.m_name, entry.m_handle);
        }
    });
}

#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
namespace {
bool smallLayoutSource(const DRW_RawDxfObject& source) {
    if (source.groups.size() > 128 || source.rawValues.size() != source.groups.size()) return false;
    std::size_t bytes = 0;
    for (std::size_t i = 0; i < source.groups.size(); ++i) {
        const auto& group = source.groups[i];
        const auto& raw = source.rawValues[i];
        const std::size_t typedBytes = !source.hasRawValues && group.type() == DRW_Variant::STRING && group.content.s
            ? group.content.s->size() : 0;
        if (raw.size() > 4096 || typedBytes > 4096 || bytes > 65536 - raw.size()) return false;
        bytes += raw.size();
        if (bytes > 65536 - typedBytes) return false;
        bytes += typedBytes;
    }
    return true;
}
bool rawCodes(const DRW_RawDxfObject& source, const std::vector<int>& codes) {
    if (!smallLayoutSource(source) || source.groups.size() != codes.size()) return false;
    for (std::size_t i = 0; i < codes.size(); ++i)
        if (source.groups[i].code() != codes[i]) return false;
    return true;
}
bool rawText(const DRW_Variant& value, const std::string& text) {
    return value.type() == DRW_Variant::STRING && value.content.s && *value.content.s == text;
}
bool rawNumber(const DRW_Variant& value, int expected) {
    return value.type() == DRW_Variant::INTEGER && value.content.i == expected;
}
bool rawDouble(const DRW_Variant& value, double expected) {
    return value.type() == DRW_Variant::DOUBLE && value.content.d == expected;
}
bool rawHandle(const DRW_Variant& value, std::uint32_t expected) {
    if (value.type() != DRW_Variant::STRING || !value.content.s || value.content.s->empty()) return false;
    try {
        std::size_t used = 0;
        const auto parsed = std::stoull(*value.content.s, &used, 16);
        return used == value.content.s->size() && parsed == expected;
    } catch (const std::exception&) { return false; }
}

bool checkedNormalPlotPair(const PreservedRecords& source, DRW::Version version, bool sourceBinary,
                           std::uint32_t excluded, std::uint32_t& plotHandle, std::uint32_t& placeholderHandle) {
    if (source.classes.size() != 2) return false;
    const auto expectedClass = [&](const char* name, const char* type) {
        return std::count_if(source.classes.begin(), source.classes.end(), [&](const auto& cls) {
            return cls.recName == name && cls.className == type && cls.appName == "ObjectDBX Classes" &&
                cls.proxyFlag == 0 && cls.instanceCount == (version == DRW::AC1015 ? 0 : 1) &&
                cls.wasaProxyFlag == 0 && cls.entityFlag == 0;
        }) == 1;
    };
    if (!expectedClass("ACDBDICTIONARYWDFLT", "AcDbDictionaryWithDefault") ||
        !expectedClass("ACDBPLACEHOLDER", "AcDbPlaceHolder")) return false;
    const auto root = std::find_if(source.rootDictEntries.begin(), source.rootDictEntries.end(),
        [](const auto& entry) { return entry.first == "ACAD_PLOTSTYLENAME"; });
    if (root == source.rootDictEntries.end() || !root->second || root->second == excluded ||
        std::count_if(source.rootDictEntries.begin(), source.rootDictEntries.end(),
            [](const auto& entry) { return entry.first == "ACAD_PLOTSTYLENAME"; }) != 1) return false;
    const auto plot = std::find_if(source.objects.begin(), source.objects.end(),
        [&](const auto& object) { return object.handle == root->second; });
    if (plot == source.objects.end() || plot->name != "ACDBDICTIONARYWDFLT" ||
        plot->m_version != version || plot->hasRawValues == sourceBinary || plot->parentHandle != 0xCu ||
        !rawCodes(*plot, {5,330,100,281,3,350,100,340}) ||
        !rawHandle(plot->groups[0], plot->handle) || !rawHandle(plot->groups[1], 0xCu) ||
        !rawText(plot->groups[2], "AcDbDictionary") || !rawNumber(plot->groups[3], 1) ||
        !rawText(plot->groups[4], "Normal") || !rawText(plot->groups[6], "AcDbDictionaryWithDefault")) return false;
    const auto placeholder = std::find_if(source.objects.begin(), source.objects.end(),
        [&](const auto& object) { return object.name == "ACDBPLACEHOLDER"; });
    if (placeholder == source.objects.end() || placeholder->m_version != version ||
        placeholder->hasRawValues == sourceBinary || !placeholder->handle || placeholder->handle == DRW::NoHandle ||
        placeholder->handle == plot->handle || placeholder->handle == excluded || placeholder->parentHandle != plot->handle ||
        !rawCodes(*placeholder, {5,330}) || !rawHandle(placeholder->groups[0], placeholder->handle) ||
        !rawHandle(placeholder->groups[1], plot->handle) || !rawHandle(plot->groups[5], placeholder->handle) ||
        !rawHandle(plot->groups[7], placeholder->handle)) return false;
    plotHandle = plot->handle;
    placeholderHandle = placeholder->handle;
    return true;
}
}

void DXFimpl::addDxfLayoutSource(const DRW_Layout& layout, const DRW_RawDxfObject& source) {
    if (!guarded("LAYOUT source", nullptr, [&] {
        if (_sourceLayouts.size() >= 3 || !smallLayoutSource(source)) {
            _unmatchedSourceLayoutBlock = true;
            return;
        }
        _sourceLayouts.emplace_back(layout, source);
    })) {
        _unmatchedSourceLayoutBlock = true;
    }
}

#ifdef LIBDXFRW_HAS_DXF_BLOCK_RECORD_SOURCE_SELECTOR
bool DXFimpl::retainDxfBlockRecordSource(const DRW_Block_Record& row) const {
    if (upperName(row.name) == "*PAPER_SPACE0") _sourceHasPaperSpace0 = true;
    const bool layoutBlock = row.name == "*Model_Space" || row.name == "*Paper_Space" ||
                             row.name == "*Paper_Space0";
    if (!layoutBlock) _hasOtherBlockRecords = true;
    return row.layoutHandle != 0 || layoutBlock;
}
#endif

void DXFimpl::addDxfBlockRecordSource(const DRW_Block_Record& block, const DRW_RawDxfObject& source) {
    if (!guarded("BLOCK_RECORD source", nullptr, [&] {
        if (block.name != "*Model_Space" && block.name != "*Paper_Space" && block.name != "*Paper_Space0") {
            _hasOtherBlockRecords = true;
            if (block.layoutHandle != 0) _unmatchedSourceLayoutBlock = true;
            return;
        }
        if (_sourceLayoutBlocks.size() >= 3 || !smallLayoutSource(source)) {
            _unmatchedSourceLayoutBlock = true;
            return;
        }
        _sourceLayoutBlocks.emplace_back(block, source);
    })) {
        _unmatchedSourceLayoutBlock = true;
    }
}

void DXFimpl::addDxfBlockBeginSource(const DRW_Block& block, const DRW_RawDxfObject& source) {
    if (!guarded("BLOCK source", nullptr, [&] {
        _activeSourceLayoutBody = -1;
        if (block.name != "*Model_Space" && block.name != "*Paper_Space" && block.name != "*Paper_Space0") return;
        if (_sourceLayoutBodies.size() >= 3 || !smallLayoutSource(source)) {
            _unmatchedSourceLayoutBlock = true;
            return;
        }
        _sourceLayoutBodies.push_back({block, DRW_Block{}, source, DRW_RawDxfObject{}, false});
        _activeSourceLayoutBody = static_cast<int>(_sourceLayoutBodies.size() - 1);
    })) {
        _activeSourceLayoutBody = -1;
        _unmatchedSourceLayoutBlock = true;
    }
}

void DXFimpl::addDxfBlockEndSource(const DRW_Block& block, const DRW_RawDxfObject& source) {
    if (!guarded("ENDBLK source", nullptr, [&] {
        if (_activeSourceLayoutBody < 0) return;
        auto& body = _sourceLayoutBodies[static_cast<std::size_t>(_activeSourceLayoutBody)];
        _activeSourceLayoutBody = -1;
        if (!smallLayoutSource(source)) { _unmatchedSourceLayoutBlock = true; return; }
        body.end = block;
        body.rawEnd = source;
        body.hasEnd = true;
    })) {
        _activeSourceLayoutBody = -1;
        _unmatchedSourceLayoutBlock = true;
    }
}

bool DXFimpl::acceptSourceLayoutGraph() {
    if (_unmatchedSourceLayoutBlock || _sourceRootHandle != 0xCu ||
        _sourceLayouts.size() != 3 || _sourceLayoutBlocks.size() != 3 || _sourceLayoutBodies.size() != 3) return false;
    std::uint32_t dictionaryHandle = 0;
    for (const auto& entry : _preserved.rootDictEntries) {
        const auto name = upperName(entry.first);
        if (name != "ACAD_LAYOUT") continue;
        if (entry.first != "ACAD_LAYOUT") return false;
        if (dictionaryHandle != 0) return false;
        dictionaryHandle = entry.second;
    }
    if (!dictionaryHandle || dictionaryHandle == DRW::NoHandle) return false;
    const auto dictionary = std::find_if(_preserved.objects.begin(), _preserved.objects.end(),
        [dictionaryHandle](const auto& value) { return value.name == "DICTIONARY" && value.handle == dictionaryHandle; });
    if (dictionary == _preserved.objects.end() ||
        std::count_if(_preserved.objects.begin(), _preserved.objects.end(),
            [dictionaryHandle](const auto& value) { return value.handle == dictionaryHandle; }) != 1 ||
        dictionary->parentHandle != 0xCu ||
        !rawCodes(*dictionary, {5,330,100,280,281,3,360,3,360,3,360}) ||
        !rawHandle(dictionary->groups[0], dictionaryHandle) || !rawHandle(dictionary->groups[1], 0xCu) ||
        !rawText(dictionary->groups[2], "AcDbDictionary") || !rawNumber(dictionary->groups[3], 1) ||
        !rawNumber(dictionary->groups[4], 1)) return false;
    // Conversion or replay to binary is limited to the complete default graph
    // and Normal plot-style pair, regardless of source encoding.
    const bool binaryCompatible = [&]() {
        if (_hasOtherBlockRecords || _preserved.objects.size() != 3 || !_preserved.entities.empty() ||
            !_preserved.sections.empty() || _preserved.classes.size() != 2 ||
            _preserved.rootDictEntries.size() != 2) return false;
        std::uint32_t plot = 0, placeholder = 0;
        return dictionary->hasRawValues != _sourceIsBinary &&
            checkedNormalPlotPair(_preserved, dictionary->m_version, _sourceIsBinary,
                                  dictionaryHandle, plot, placeholder);
    }();
    if (_sourceIsBinary && !binaryCompatible) return false;
    const char* names[] = {"Model", "Layout1", "Layout2"};
    const char* blockNames[] = {"*Model_Space", "*Paper_Space", "*Paper_Space0"};
    std::vector<DRW_Layout> layouts;
    std::vector<DRW_Block_Record> blocks;
    std::vector<DRW_RawDxfObject> rawLayouts;
    for (int i = 0; i < 3; ++i) {
        const auto layout = std::find_if(_sourceLayouts.begin(), _sourceLayouts.end(),
            [&](const auto& item) { return item.first.name == names[i]; });
        const auto block = std::find_if(_sourceLayoutBlocks.begin(), _sourceLayoutBlocks.end(),
            [&](const auto& item) { return item.first.name == blockNames[i]; });
        const auto body = std::find_if(_sourceLayoutBodies.begin(), _sourceLayoutBodies.end(),
            [&](const auto& item) { return item.begin.name == blockNames[i]; });
        if (layout == _sourceLayouts.end() || block == _sourceLayoutBlocks.end() || body == _sourceLayoutBodies.end()) return false;
        const auto& l = layout->first; const auto& ls = layout->second;
        const auto& b = block->first; const auto& bs = block->second;
        std::vector<int> layoutCodes{5,330,100,1,2,4,6,40,41,42,43,44,45,46,47,48,49,140,141,142,143,70,72,73,74,7,75};
        if (ls.m_version > DRW::AC1015) layoutCodes.insert(layoutCodes.end(), {76,77,78});
        const std::vector<int> suffix{147,148,149,100,1,70,71,10,20,11,21,12,22,32,14,24,34,15,25,35,146,13,23,33,16,26,36,17,27,37,76,330};
        layoutCodes.insert(layoutCodes.end(), suffix.begin(), suffix.end());
        std::vector<int> blockCodes{5,330,100,100,2,340};
        if (ls.m_version > DRW::AC1018) blockCodes.insert(blockCodes.end(), {70,280,281});
        if (ls.m_version < DRW::AC1015 || ls.m_version != bs.m_version || ls.m_version != dictionary->m_version ||
            ls.name != "LAYOUT" || bs.name != "BLOCK_RECORD" ||
            ls.hasRawValues == _sourceIsBinary || bs.hasRawValues == _sourceIsBinary ||
            !rawCodes(ls, layoutCodes) || !rawCodes(bs, blockCodes) ||
            !l.handle || l.handle == DRW::NoHandle || l.parentHandle != dictionaryHandle || l.tabOrder != i ||
            !l.pageSetupName.empty() || !l.printerConfig.empty() || !l.paperSize.empty() ||
            !l.plotViewName.empty() || !l.currentStyleSheet.empty() ||
            !b.handle || b.handle == DRW::NoHandle || b.layoutHandle != l.handle ||
            l.paperSpaceBlockRecordHandle.ref != b.handle ||
            ls.handle != l.handle || ls.parentHandle != dictionaryHandle || bs.handle != b.handle || bs.parentHandle != 1u ||
            !rawHandle(ls.groups[0], l.handle) || !rawHandle(ls.groups[1], dictionaryHandle) ||
            !rawText(ls.groups[2], "AcDbPlotSettings") || !rawText(ls.groups[ls.groups.size()-29], "AcDbLayout") ||
            !rawHandle(ls.groups.back(), b.handle) || !rawHandle(bs.groups[0], b.handle) || !rawHandle(bs.groups[1], 1u) ||
            !rawText(bs.groups[2], "AcDbSymbolTableRecord") || !rawText(bs.groups[3], "AcDbBlockTableRecord") ||
            !rawText(bs.groups[4], blockNames[i]) || !rawHandle(bs.groups[5], l.handle) ||
            (blockCodes.size() == 9 && (!rawNumber(bs.groups[6], 0) || !rawNumber(bs.groups[7], 1) || !rawNumber(bs.groups[8], 0))) ||
            !rawText(dictionary->groups[5 + i*2], names[i]) || !rawHandle(dictionary->groups[6 + i*2], l.handle)) return false;
        const auto& begin = body->rawBegin; const auto& end = body->rawEnd;
        const std::uint32_t beginHandle = i == 0 ? 0x20u : i == 1 ? 0x1Cu : b.handle + 1u;
        const std::uint32_t endHandle = i == 0 ? 0x21u : i == 1 ? 0x1Du : b.handle + 2u;
        if (!body->hasEnd || b.handle > DRW::NoHandle - 2u || begin.name != "BLOCK" || end.name != "ENDBLK" ||
            begin.m_version != ls.m_version || end.m_version != ls.m_version ||
            begin.hasRawValues == _sourceIsBinary || end.hasRawValues == _sourceIsBinary || begin.handle != beginHandle ||
            end.handle != endHandle || begin.parentHandle != b.handle || end.parentHandle != b.handle) return false;
        if (i < 2) {
            if (!rawCodes(begin, {5,330,100,8,100,2,70,10,20,30,3,1}) ||
                !rawCodes(end, {5,330,100,8,100}) || !rawHandle(begin.groups[0], beginHandle) ||
                !rawHandle(begin.groups[1], b.handle) || !rawText(begin.groups[2], "AcDbEntity") ||
                !rawText(begin.groups[3], "0") || !rawText(begin.groups[4], "AcDbBlockBegin") ||
                !rawText(begin.groups[5], blockNames[i]) || !rawNumber(begin.groups[6], 0) ||
                !rawDouble(begin.groups[7], 0) || !rawDouble(begin.groups[8], 0) || !rawDouble(begin.groups[9], 0) ||
                !rawText(begin.groups[10], blockNames[i]) || !rawText(begin.groups[11], "") ||
                !rawHandle(end.groups[0], endHandle) || !rawHandle(end.groups[1], b.handle) ||
                !rawText(end.groups[2], "AcDbEntity") || !rawText(end.groups[3], "0") ||
                !rawText(end.groups[4], "AcDbBlockEnd")) return false;
        } else {
            if (!rawCodes(begin, {5,330,100,8,6,62,370,100,2,70,10,20,3,1}) ||
                !rawCodes(end, {5,330,100,8,6,62,370,100}) || !rawHandle(begin.groups[0], beginHandle) ||
                !rawHandle(begin.groups[1], b.handle) || !rawText(begin.groups[2], "AcDbEntity") ||
                !rawText(begin.groups[3], "0") || !rawText(begin.groups[4], "BYLAYER") ||
                !rawNumber(begin.groups[5], 256) || !rawNumber(begin.groups[6], -1) ||
                !rawText(begin.groups[7], "AcDbBlockBegin") || !rawText(begin.groups[8], blockNames[i]) ||
                !rawNumber(begin.groups[9], 0) || !rawDouble(begin.groups[10], 0) || !rawDouble(begin.groups[11], 0) ||
                !rawText(begin.groups[12], blockNames[i]) || !rawText(begin.groups[13], "") ||
                !rawHandle(end.groups[0], endHandle) || !rawHandle(end.groups[1], b.handle) ||
                !rawText(end.groups[2], "AcDbEntity") || !rawText(end.groups[3], "0") ||
                !rawText(end.groups[4], "BYLAYER") || !rawNumber(end.groups[5], 256) ||
                !rawNumber(end.groups[6], -1) || !rawText(end.groups[7], "AcDbBlockEnd")) return false;
            _preserved.layoutExtraBlockBegin = body->begin;
            _preserved.layoutExtraBlockEnd = body->end;
        }
        layouts.push_back(l); blocks.push_back(b); rawLayouts.push_back(ls);
    }
    std::set<std::uint32_t> identities{0xCu, dictionaryHandle};
    if (identities.size() != 2) return false;
    for (const auto& layout : layouts) if (!identities.insert(layout.handle).second) return false;
    for (const auto& block : blocks) if (!identities.insert(block.handle).second) return false;
    for (const auto& body : _sourceLayoutBodies)
        if (!identities.insert(body.begin.handle).second || !identities.insert(body.end.handle).second) return false;
    for (const auto& object : _preserved.objects)
        if (object.handle != dictionaryHandle && identities.count(object.handle)) return false;
    for (const auto& entity : _preserved.entities)
        if (identities.count(entity.handle)) return false;
    _preserved.layoutDictionary = dictionaryHandle;
    _preserved.layouts = std::move(layouts);
    _preserved.layoutBlockRecords = std::move(blocks);
    _preserved.objects.insert(_preserved.objects.end(), rawLayouts.begin(), rawLayouts.end());
    _preserved.checkedLayoutGraph = true;
    _preserved.checkedBinaryLayoutGraph = binaryCompatible;
    _loss.droppedByType.erase("LAYOUT");
    return true;
}
#endif

void DXFimpl::attachPreservedRecords(bool completeRead) {
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    if (completeRead) acceptSourceLayoutGraph();
#else
    (void)completeRead;
#endif
    // The header travels even when there is nothing else to carry: a drawing's
    // units are worth keeping on their own.
    const bool hasHeader = !_header.acadVersion.empty() || _header.hasInsUnits || _header.hasMeasurement;
    // Retain an empty shelf too: it distinguishes an imported headerless DXF
    // from a new document when deciding whether a default block is synthetic.
    auto preserved = std::make_shared<PreservedRecords>(_preserved);
#if defined(LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH) && defined(LIBDXFRW_HAS_DXF_BLOCK_RECORD_SOURCE_SELECTOR)
    preserved->mayOmitSyntheticPaperSpace0 = completeRead && !_sourceHasPaperSpace0;
#endif
    preserved->hasHeader = hasHeader;
    preserved->insUnitsCode = _header.insUnitsCode;
    preserved->hasInsUnits = _header.hasInsUnits;
    preserved->measurement = _header.measurement;
    preserved->hasMeasurement = _header.hasMeasurement;
    preserved->lineTypeScale = _header.lineTypeScale;

    // The revision comes from the records themselves: libdxfrw stamps each one
    // with the version it was captured from, which is the only revision it can
    // safely be replayed into.
    // Every kind is asked, not just objects and sections: libdxfrw stamps raw
    // entities too, and a capture can be made of nothing else. Leaving those
    // out left the version at UNKNOWNV, which matches no export target, so the
    // replay was skipped even for a save straight back into the revision the
    // file was read as -- the records were dropped and the drop was reported.
    for (const auto& object : preserved->objects) {
        if (object.m_version != DRW::UNKNOWNV) {
            preserved->version = object.m_version;
            break;
        }
    }
    if (preserved->version == DRW::UNKNOWNV) {
        for (const auto& entity : preserved->entities) {
            if (entity.m_version != DRW::UNKNOWNV) {
                preserved->version = entity.m_version;
                break;
            }
        }
    }
    if (preserved->version == DRW::UNKNOWNV) {
        for (const auto& section : preserved->sections) {
            if (section.m_version != DRW::UNKNOWNV) {
                preserved->version = section.m_version;
                break;
            }
        }
    }
    // Nothing carried one -- a classes-only capture, or a build of libdxfrw
    // that does not stamp them. The revision the header stated is the next best
    // answer, and it is the revision the file was read as.
    if (preserved->version == DRW::UNKNOWNV && !_header.acadVersion.empty()) {
        bool recognised = false;
        const lc::persistence::File::Type headerType =
            lc::persistence::File::typeForAcadVersion(_header.acadVersion, &recognised);
        if (recognised) {
            preserved->version = dxfRevisionForType(headerType);
        }
    }
    _document->addDocumentMetaType(preserved);

    LOG_DEBUG << "Preserved " << preserved->objects.size() << " object(s), "
              << preserved->entities.size() << " entity/entities and "
              << preserved->sections.size() << " section(s) this build does not model";
}

void DXFimpl::attachDwgProvenance() {
    auto preserved = std::make_shared<PreservedRecords>();
    preserved->sourceWasDwg = true;
    preserved->sourceLayerColors = _preserved.sourceLayerColors;
    preserved->hasSourceLayouts = _preserved.hasSourceLayouts;
    _document->addDocumentMetaType(preserved);
}

/** The records a previous read of this document set aside, if any. */
static std::shared_ptr<const lc::persistence::PreservedRecords> preservedOn(
    const std::shared_ptr<lc::storage::Document>& document) {
    if (document == nullptr) {
        return nullptr;
    }

    const auto all = document->allMetaTypes();
    const auto found = all.find(lc::persistence::PreservedRecords::kId);
    if (found == all.end()) {
        return nullptr;
    }

    return std::dynamic_pointer_cast<const lc::persistence::PreservedRecords>(found->second);
}

namespace {

/// The first code-5 handle libdxfrw's allocator will mint. Everything the DXF
/// codec writes as a fixed structural literal -- table heads, LAYER "0", the
/// *Model_Space and *Paper_Space blocks, the root dictionary -- is below it,
/// which is what makes it a sound test for "this handle may be written twice".
/// See HandleAllocator::m_next in third_party/libdxfrw/src/handle_allocator.h.
constexpr std::uint32_t kFirstMintedHandle = 0x30;

}  // namespace

#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
std::vector<dxfRW::DxfLayoutOutputLinks> DXFimpl::defaultLayoutOutputLinks() const {
    if (_defaultLayouts.size() != 3) return {};
    return {
        {"Model", "*Model_Space", dxfW->remapHandle(_defaultLayouts[0].handle), 0x1Fu, 0},
        {"Layout1", "*Paper_Space", dxfW->remapHandle(_defaultLayouts[1].handle), 0x1Eu, 1},
        {"Layout2", "*Paper_Space0", dxfW->remapHandle(_defaultLayouts[2].handle),
         dxfW->getBlockRecordHandleToWrite("*Paper_Space0"), 2}
    };
}

bool DXFimpl::prepareDefaultLayoutGraph() {
    _defaultLayouts.clear();
    _defaultLayoutDictionary = 0;
    _defaultLayoutWriteOk = true;
    _defaultLayoutsFromSource = _replay && _replay->checkedLayoutGraph;
    if (_exportVersion < DRW::AC1015) return true;
    // A new document or a checked binary source can write the graph in binary.
    if (lc::persistence::File::isBinaryType(_exportType)) {
        if (_shelf && !_defaultLayoutsFromSource) return true;
        const auto primary = _document->blockByName("*Paper_Space");
        const bool primaryHasContent = primary && !_document->entitiesByBlock(primary).asVector().empty();
        if (!_defaultLayoutsFromSource && !primaryHasContent && !hasUnsafeAdditionalPaperSpace()) return true;
    }
    if (!_defaultLayoutsFromSource &&
        (_replay || (_shelf && (_shelf->sourceWasDwg || _shelf->hasSourceLayouts || _shelf->total() != 0 ||
                                 !_shelf->rootDictEntries.empty())))) return true;

    // Only synthesize defaults when the document has the two kernel-created
    // paper spaces and no additional paper-space identity to guess about.
    unsigned paper = 0, paper0 = 0;
    const std::string prefix = "*PAPER_SPACE";
    for (const auto& block : _document->blocks()) {
        const auto& name = block->name();
        if (isModelSpaceName(name)) {
            const auto& base = block->base();
            if (name != "*Model_Space" || block->anonymous() || base.x() != 0 || base.y() != 0 || base.z() != 0 ||
                std::dynamic_pointer_cast<const lc::meta::CustomEntityStorage>(block) ||
                !_document->entitiesByBlock(block).asVector().empty()) return !_defaultLayoutsFromSource;
            continue;
        }
        const auto upper = upperName(name);
        if (upper.compare(0, prefix.size(), prefix) != 0) continue;
        if (name == "*Paper_Space") {
            const auto& base = block->base();
            if (block->anonymous() || base.x() != 0 || base.y() != 0 || base.z() != 0 ||
                std::dynamic_pointer_cast<const lc::meta::CustomEntityStorage>(block)) return !_defaultLayoutsFromSource;
            ++paper;
        }
        else if (name == "*Paper_Space0") ++paper0;
        else return !_defaultLayoutsFromSource;
    }
    if (paper != 1 || paper0 != 1) return !_defaultLayoutsFromSource;

    if (_defaultLayoutsFromSource) {
        _defaultLayouts = _replay->layouts;
        _defaultLayoutDictionary = dxfW->remapHandle(_replay->layoutDictionary);
        dxfW->setSourceBlockRecords(_replay->layoutBlockRecords);
    } else {
        const auto claim = [this](std::uint32_t& handle) {
            handle = dxfW->highWaterHandle();
            return handle != 0 && handle != DRW::NoHandle && dxfW->reserveHandle(handle);
        };
        if (!claim(_defaultLayoutDictionary)) return false;
        DRW_Dictionary dictionary;
        dictionary.handle = _defaultLayoutDictionary;
        dictionary.parentHandle = 0xCu;
        dictionary.hardOwner = 1;
        dictionary.cloning = 1;
        std::vector<DRW_Block_Record> records;
        const char* names[] = {"Model", "Layout1", "Layout2"};
        const char* blocks[] = {"*Model_Space", "*Paper_Space", "*Paper_Space0"};
        for (int i = 0; i < 3; ++i) {
            std::uint32_t layoutHandle = 0, sourceBlockHandle = 0;
            if (!claim(layoutHandle) || !claim(sourceBlockHandle)) return false;
            DRW_Layout layout;
            layout.handle = layoutHandle;
            layout.parentHandle = _defaultLayoutDictionary;
            layout.name = names[i];
            layout.tabOrder = i;
            layout.paperSpaceBlockRecordHandle.ref = sourceBlockHandle;
            _defaultLayouts.push_back(layout);
            DRW_Block_Record record;
            record.handle = sourceBlockHandle;
            record.name = blocks[i];
            record.layoutHandle = layoutHandle;
            records.push_back(record);
            dictionary.m_entries.push_back({names[i], layoutHandle});
        }
        dxfW->setRootDictEntries({{"ACAD_LAYOUT", dxfW->toHexStrHandle(_defaultLayoutDictionary)}});
        dxfW->setNamedDictObjects({dictionary});
        dxfW->setSourceBlockRecords(records);
    }
    dxfW->setDxfWriteRecordInventoryEnabled(true);
    return dxfW->setDxfTablesWriteInspector([this](const auto& view) {
        return dxfW->inspectDxfLayoutBlockRecords(view, defaultLayoutOutputLinks());
    }) && dxfW->setDxfObjectsWriteInspector([this](const auto& view) {
        return dxfW->inspectDxfLayoutObjects(view, _defaultLayoutDictionary,
                                             defaultLayoutOutputLinks());
    });
}

bool DXFimpl::finalizeDxfWrite() {
    if (_defaultLayouts.empty()) return true;
    if (!_defaultLayoutWriteOk ||
        !dxfW->dxfWriteRecordHasIdentity(_defaultLayoutDictionary, "DICTIONARY", "OBJECTS") ||
        !dxfW->dxfWriteRecordHasOwner(_defaultLayoutDictionary, 0xCu)) return false;
    for (const auto& link : defaultLayoutOutputLinks()) {
        if (!dxfW->dxfWriteRecordHasIdentity(link.layout, "LAYOUT", "OBJECTS") ||
            !dxfW->dxfWriteRecordHasOwner(link.layout, _defaultLayoutDictionary) ||
            !dxfW->dxfWriteRecordHasIdentity(link.blockRecord, "BLOCK_RECORD", "TABLES")) return false;
    }
    return true;
}

#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
bool DXFimpl::writeDxfFixedPaperSpaceContents() {
    if (_defaultLayouts.empty()) return true;
    for (const auto& block : _document->blocks()) {
        if (block->name() != "*Paper_Space") continue;
        for (const auto& entity : _document->entitiesByBlock(block).asVector()) writeEntity(entity);
        return true;
    }
    return false;
}
#endif
#endif

bool DXFimpl::hasUnsafeAdditionalPaperSpace() const {
    const std::string prefix = "*PAPER_SPACE";
    const auto blocks = _document->blocks();
    std::set<std::string> extraNames;
    for (const auto& block : blocks) {
        const auto name = upperName(block->name());
        if (name == prefix || name.compare(0, prefix.size(), prefix) != 0) continue;
        extraNames.insert(name);
        const auto& base = block->base();
        if (block->anonymous() || base.x() != 0 || base.y() != 0 || base.z() != 0 ||
            std::dynamic_pointer_cast<const lc::meta::CustomEntityStorage>(block) ||
            !_document->entitiesByBlock(block).asVector().empty()) return true;
    }
    if (extraNames.empty()) return false;
    const auto referencesExtra = [&](const auto& entities) {
        for (const auto& entity : entities) {
            const auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(entity);
            if (insert && insert->displayBlock() && extraNames.count(upperName(insert->displayBlock()->name())))
                return true;
        }
        return false;
    };
    if (referencesExtra(_document->entityContainer().asVector())) return true;
    for (const auto& owner : blocks)
        if (referencesExtra(_document->entitiesByBlock(owner).asVector())) return true;
    return false;
}

#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
bool DXFimpl::hasCheckedBinaryImageEntities() const {
    for (const auto& entity : _document->entityContainer().asVector()) {
        if (!entity || entity->block() ||
            (!std::dynamic_pointer_cast<const lc::entity::Line>(entity) &&
             !std::dynamic_pointer_cast<const lc::entity::Image>(entity))) return false;
    }
    for (const auto& block : _document->blocks()) {
        if (!block || (block->name() != "*Model_Space" && block->name() != "*Paper_Space" &&
                       block->name() != "*Paper_Space0") ||
            !_document->entitiesByBlock(block).asVector().empty()) return false;
    }
    return true;
}

bool DXFimpl::hasCheckedImageGraphWithoutPaper() const {
    if (!_shelf || _shelf->rootDictSourceAmbiguous ||
        _shelf->rootDictSourceHandle != kRootDictionaryHandle ||
        !_shelf->entities.empty() || !_shelf->sections.empty() ||
        _shelf->objects.size() > 512 || _shelf->rootDictEntries.empty() ||
        _shelf->rootDictEntries.size() > 128) return false;
    std::uint32_t plot = 0, placeholder = 0;
    if (!_shelf->classes.empty() &&
        !checkedNormalPlotPair(*_shelf, _shelf->version, true, 0, plot, placeholder)) return false;
    bool hasImages = false;
    const auto skipped = skippedImageSourceHandles(hasImages, _shelf.get());
    if (skipped.empty()) return false;
    if (_replay != _shelf) {
        // When binary output drops the raw graph, admit only standard blocks
        // and model-space writers whose fields cannot replay a source edge.
        if (_replay || !lc::persistence::File::isBinaryType(_exportType) ||
            _shelf->version != _exportVersion || !hasCheckedBinaryImageEntities()) return false;
    }
    bool sameSource = true;
    visitDocumentImages([&](const lc::entity::Image_CSPtr& image) {
        const auto source = image->metaInfo<ImageDisplaySource>(ImageDisplaySource::key());
        if (!source || source->sourceIdentity != _shelf->imageSourceIdentity) sameSource = false;
    });
    if (!sameSource) return false;
    std::unordered_map<std::uint32_t, const DRW_RawDxfObject*> objects;
    for (const auto& object : _shelf->objects) {
        if (object.handle == 0 || object.handle == DRW::NoHandle || object.handle == kRootDictionaryHandle ||
            object.groups.size() > 512 || !objects.emplace(object.handle, &object).second) return false;
    }
    std::unordered_set<std::uint32_t> roots;
    std::set<std::string> rootNames;
    for (const auto& entry : _shelf->rootDictEntries) {
        const auto object = objects.find(entry.second);
        if (entry.first.empty() || !rootNames.insert(upperName(entry.first)).second ||
            !roots.insert(entry.second).second || object == objects.end() ||
            (entry.second == plot ? entry.first != "ACAD_PLOTSTYLENAME" :
             object->second->name != "DICTIONARY")) return false;
    }
    std::unordered_set<std::uint32_t> referenced;
    for (const auto& object : _shelf->objects) {
        if (object.handle == plot || object.handle == placeholder) continue;
        if (skipsSourceImageObject(object, hasImages, _shelf.get())) {
            if (!skipped.count(object.handle)) return false;
            continue;
        }
        if (object.name == "XRECORD") continue;
        if (object.parentHandle != kRootDictionaryHandle) return false;
        std::vector<std::pair<std::size_t, std::uint32_t>> entries;
        if (!ordinaryDictionaryEntries(object, entries)) return false;
        // Binary outputs can contain an unreferenced empty source dictionary
        // after regenerating the IMAGE dictionary. Keep that bounded carrier.
        if (!roots.count(object.handle) && (object.hasRawValues || !entries.empty())) return false;
        std::set<std::string> names;
        std::unordered_set<std::uint32_t> targets;
        for (const auto& entry : entries) {
            if (!names.insert(upperName(*object.groups[entry.first].content.s)).second ||
                !targets.insert(entry.second).second) return false;
            if (skipped.count(entry.second)) continue;
            const auto target = objects.find(entry.second);
            if (target == objects.end() || !simpleTextXRecord(*target->second, object.handle) ||
                !referenced.insert(entry.second).second) return false;
        }
    }
    for (const auto& object : _shelf->objects)
        if (object.name == "XRECORD" && !referenced.count(object.handle)) return false;
    return true;
}

bool DXFimpl::canTranscodeCheckedImageGraph() const {
    if (!_shelf || _replay != _shelf || _shelf->sourceWasDwg ||
        !_shelf->mayOmitSyntheticPaperSpace0 || _shelf->hasSourceLayouts ||
        !hasCheckedBinaryImageEntities() || !hasCheckedImageGraphWithoutPaper()) return false;
    // Typed replay admits bounded printable ASCII; captured and decoded
    // spellings must agree when present. Escaping and legacy code pages need
    // their own round-trip proof.
    const auto safeText = [](const std::string& value) {
        return value.size() <= 255 && std::all_of(value.begin(), value.end(), [](unsigned char c) {
            return c >= 0x20 && c <= 0x7e && c != '\\';
        });
    };
    for (const auto& entry : _shelf->rootDictEntries) {
        const auto name = upperName(entry.first);
        if (!safeText(entry.first) || name == "ACAD_GROUP" ||
            (name == "ACAD_PLOTSTYLENAME" && _shelf->classes.empty()) ||
            name == "ACAD_LAYOUT") return false;
    }
    bool hasImages = false;
    skippedImageSourceHandles(hasImages, _shelf.get());
    bool sourceEncodingKnown = false, sourceHasRawValues = false;
    for (const auto& object : _shelf->objects) {
        if (skipsSourceImageObject(object, hasImages, _shelf.get())) continue;
        if (!sourceEncodingKnown) {
            sourceEncodingKnown = true;
            sourceHasRawValues = object.hasRawValues;
        }
        if (object.hasRawValues != sourceHasRawValues || object.m_version != _exportVersion ||
            object.groups.size() != object.rawValues.size()) return false;
        for (std::size_t i = 0; i < object.groups.size(); ++i) {
            const auto& group = object.groups[i];
            if (group.type() == DRW_Variant::STRING) {
                if (!group.content.s || !safeText(*group.content.s) ||
                    (sourceHasRawValues ? *group.content.s != object.rawValues[i] : !object.rawValues[i].empty())) return false;
            } else if (group.type() == DRW_Variant::INTEGER) {
                if (group.i_val() < std::numeric_limits<std::int16_t>::min() ||
                    group.i_val() > std::numeric_limits<std::int16_t>::max()) return false;
            } else return false;
        }
    }
    return true;
}

bool DXFimpl::mayOmitUnusedDefaultPaper() const {
    const bool sourceAllowsOmission = !_shelf ||
        (!_shelf->sourceWasDwg && _shelf->mayOmitSyntheticPaperSpace0 &&
         !_shelf->hasSourceLayouts &&
         ((_shelf->total() == 0 && _shelf->rootDictEntries.empty() && _shelf->classes.empty()) ||
          hasCheckedImageGraphWithoutPaper()));
    return sourceAllowsOmission && _defaultLayouts.empty() && !hasUnsafeAdditionalPaperSpace();
}
#endif

bool DXFimpl::writeDXF(const std::string& filename, lc::persistence::File::Type type) {
    // Each attempt reports its own losses, including after an early refusal.
    _loss = LossSummary{};
    _mtextsAsText = 0;
    _mpolygonsWrittenAsHatch = 0;
    _dimensionBlocks.clear();
    _emittedImageDefinitions.clear();
#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
    _imageSourceGenerations.clear();
    _plannedImageDefinitions.clear();
    _imageXdataPartialSources.clear();
    _imageXdataAppIds.clear();
#ifdef LC3_HAS_SOURCE_IMAGE_DICTIONARY
    _boundSourceImageDictionary = 0;
#endif
#endif
    _writeFailureReason.clear();

    //Default setting
    // Initialised: the switch below is exhaustive over the DXF and DXB targets,
    // but a DWG enumerator reaches it too, and reading this uninitialised was
    // undefined behaviour that landed on a build-dependent revision.
    DRW::Version exportVersion = DRW::UNKNOWNV;

    switch(type) {
    case lc::persistence::File::LIBDXFRW_DXF_R12:
    case lc::persistence::File::LIBDXFRW_DXB_R12:
        exportVersion = DRW::AC1009;
        break;
    case lc::persistence::File::LIBDXFRW_DXF_R14:
    case lc::persistence::File::LIBDXFRW_DXB_R14:
        exportVersion = DRW::AC1014;
        break;
    case lc::persistence::File::LIBDXFRW_DXF_R2000:
    case lc::persistence::File::LIBDXFRW_DXB_R2000:
        exportVersion = DRW::AC1015;
        break;
    case lc::persistence::File::LIBDXFRW_DXF_R2004:
    case lc::persistence::File::LIBDXFRW_DXB_R2004:
        exportVersion = DRW::AC1018;
        break;
    case lc::persistence::File::LIBDXFRW_DXF_R2007:
    case lc::persistence::File::LIBDXFRW_DXB_R2007:
        exportVersion = DRW::AC1021;
        break;
    case lc::persistence::File::LIBDXFRW_DXF_R2010:
    case lc::persistence::File::LIBDXFRW_DXB_R2010:
        exportVersion = DRW::AC1024;
        break;
    case lc::persistence::File::LIBDXFRW_DXF_R2013:
    case lc::persistence::File::LIBDXFRW_DXB_R2013:
        exportVersion = DRW::AC1027;
        break;
    case lc::persistence::File::LIBOPENCAD_DWG:
    case lc::persistence::File::LIBDXFRW_DWG_IMPORT:
        // Neither is a writable target: LibreCAD reads DWG and does not write
        // it. LIBOPENCAD_DWG is refused by File::save before this is reached,
        // but LIBDXFRW_DWG_IMPORT can reach File::exportFile because its
        // library is libdxfrw. Refuse both here rather than letting a
        // DWG-labelled file claim an arbitrary DXF revision.
        LOG_ERROR << "No DXF revision for file type " << static_cast<int>(type);
        return false;
    }

    if(exportVersion == DRW::UNKNOWNV) {
        LOG_ERROR << "Unhandled file type " << static_cast<int>(type);
        return false;
    }

    // R12/R14 have no layout object to own additional paper-space blocks.
    // Peers can remove those orphan blocks together with their contents. Refuse
    // before opening the destination when such a block carries data or a link.
    if (exportVersion <= DRW::AC1014 && hasUnsafeAdditionalPaperSpace()) {
        _writeFailureReason = "This DXF revision cannot safely retain additional paper-space contents or references; save as R2000 or newer.";
        return false;
    }
    dxfW = new dxfRW(filename.c_str());

    // Was `< LIBDXFRW_DXB_R2013`, so the last binary target wrote ASCII.
    const bool isBinary = lc::persistence::File::isBinaryType(type);

    _exportVersion = exportVersion;
    _exportType = type;

    // Put back whatever the read set aside, if this is the same revision it
    // came from. A record valid in R2013 has no defined meaning in R12, and
    // re-emitting it there would produce a file that claims to be R12 and is
    // not -- so a down-convert drops them, and says how many.
    // The shelf itself, before any of the replay gating below: extended entity
    // data is written from typed variants and is carried by every revision, so
    // none of the reasons a raw record cannot be replayed apply to it.
    _shelf = preservedOn(_document);

    _replay = _shelf;
    if (_replay != nullptr && _replay->total() == 0) {
        // Carries only the header, which is revision independent and handled by
        // writeHeader. Nothing to replay, and nothing to report as unreplayed:
        // a zero-count entry in the loss summary is still a loss to every
        // caller that checks whether the summary is empty.
        _replay = nullptr;
    }

    if (_replay != nullptr) {
        // Replay is only attempted where it is known to be faithful.
        //
        //  * Same revision: a record valid in R2013 has no defined meaning in
        //    R12, and re-emitting it there would produce a file that claims to
        //    be R12 and is not.
        //  * General ASCII replay requires captured source spellings.
        //  * Typed cross-encoding is restricted to the checked default layout
        //    graph and Normal plot-style pair.
        const bool sameRevision = _replay->version == exportVersion;
        const bool verbatimAvailable =
            std::all_of(_replay->objects.begin(), _replay->objects.end(),
                        [](const DRW_RawDxfObject& object) { return object.hasRawValues; });

        bool checkedEncodingGraph = _replay->checkedBinaryLayoutGraph;
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        if (sameRevision && !checkedEncodingGraph && (isBinary || !verbatimAvailable))
            checkedEncodingGraph = canTranscodeCheckedImageGraph();
#endif
        if (!sameRevision || (isBinary && !checkedEncodingGraph) ||
            (!isBinary && !verbatimAvailable && !checkedEncodingGraph)) {
            const char* reason = !sameRevision ? "the revision differs from the one it was read as"
                                 : isBinary ? "binary DXF cannot carry them"
                                 : "they were read from a binary file and have no verbatim form";
            LOG_WARNING << filename << " is being written without "
                        << _replay->total() << " record(s) this build does not model: " << reason;
            // Only when something is actually dropped: an entry of zero used to
            // be reported to the user as a partial save.
            if(const std::size_t dropped = _replay->total(); dropped > 0) {
                _loss.droppedByType["unmodelled records"] += dropped;
            }
            _replay = nullptr;
        }
    }

    if (_replay != nullptr) {
        // A typed source IMAGEDEF is not in the raw replay list. Reserve its
        // identity too, but never remap an ambiguous source collision into an
        // unrelated preserved record or the regenerated root/group handles.
        const auto& imageDefinitions = _replay->sourceImageDefinitionHandles;
        const bool fixedCollision = !imageDefinitions.empty() &&
            (_replay->rootDictSourceAmbiguous || imageDefinitions.count(_replay->rootDictSourceHandle) ||
             imageDefinitions.count(_replay->groupDictSourceHandle));
        const bool objectCollision = std::any_of(_replay->objects.begin(), _replay->objects.end(),
            [&](const DRW_RawDxfObject& object) {
                return object.name != "IMAGEDEF" && imageDefinitions.count(object.handle);
            });
        const bool entityCollision = std::any_of(_replay->entities.begin(), _replay->entities.end(),
            [&](const DRW_RawDxfObject& entity) { return imageDefinitions.count(entity.handle); });
        if (fixedCollision || objectCollision || entityCollision) {
            _writeFailureReason = "An image definition shares a source handle with another object; preserve the original and repair the drawing.";
            delete dxfW;
            dxfW = nullptr;
            return false;
        }
        // Every preserved handle is claimed before the write starts, so the
        // handles the typed writers mint cannot collide with one being
        // re-emitted verbatim.
        //
        // Reserving is not enough on its own. It constrains only the handles
        // the allocator mints; the codec also writes a fixed set of structural
        // handles as literals -- the table heads, layer "0", the *Model_Space
        // and *Paper_Space blocks, the root dictionary -- and those go out
        // whatever we reserve. A preserved record that arrived carrying one of
        // them was re-emitted under the same code 5 as the codec's own record,
        // so the file went out with two objects claiming one identity and every
        // 330/350 reference to it ambiguous. The save reported success. Four of
        // 146 ordinary ASCII DXFs in a real corpus came out that way.
        //
        // The whole fixed set is below 0x30, which the allocator documents as
        // the first handle it will ever mint, so that is the test: anything
        // below it gets a fresh handle and the codec rewrites every reference
        // along with the record's own code 5. A handle below 0x30 that would
        // not actually have collided is moved too, which costs nothing -- the
        // identity of a preserved record is only ever reached through those
        // references.
        // Two passes, and the order is the point. A fresh handle comes from the
        // top of the reserved range, so every handle being kept verbatim has
        // to be reserved before any is handed out -- otherwise a record early
        // in the walk is moved onto a handle a record later in the walk holds
        // for itself, and the file goes out with the collision this is here to
        // prevent.
        std::map<std::uint32_t, std::uint32_t> remap;
        std::vector<std::uint32_t> needFresh;
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        if (_replay->checkedLayoutGraph)
            for (const auto& block : _replay->layoutBlockRecords)
                if (!dxfW->reserveHandle(block.handle)) { delete dxfW; dxfW = nullptr; return false; }
        if (_replay->checkedLayoutGraph &&
            (!dxfW->reserveHandle(_replay->layoutExtraBlockBegin.handle) ||
             !dxfW->reserveHandle(_replay->layoutExtraBlockEnd.handle))) {
            delete dxfW; dxfW = nullptr; return false;
        }
#endif
        const auto claim = [&](std::uint32_t handle) {
            if (handle == 0) {
                return;
            }
            if (handle >= kFirstMintedHandle) {
                dxfW->reserveHandle(handle);
                return;
            }
            needFresh.push_back(handle);
        };

        for (const auto& object : _replay->objects) {
            claim(object.handle);
        }
        for (const auto& entity : _replay->entities) {
            claim(entity.handle);
        }
        for (const auto handle : _replay->sourceImageDefinitionHandles) claim(handle);

        for (const std::uint32_t handle : needFresh) {
            if (remap.count(handle) != 0) {
                continue;
            }
            // highWaterHandle(), not allocHandle(): write() resets the handles
            // minted by the previous attempt, and one taken here would be
            // forgotten and minted again during the write -- which just moves
            // the collision. A reservation is what survives the reset.
            const std::uint32_t fresh = dxfW->highWaterHandle();
            if (dxfW->reserveHandle(fresh)) {
                remap[handle] = fresh;
            }
        }

        if (!remap.empty()) {
            LOG_INFO << "remapping " << remap.size()
                     << " preserved handle(s) away from the codec's own structural handles";
            dxfW->setHandleRemap(remap);
        }

        // Put the root dictionary's entries back, for the dictionaries this
        // save really does re-emit.
        //
        // libdxfrw regenerates the root NamedObjectsDictionary holding only
        // ACAD_GROUP and routes every other named dictionary through the raw
        // net, so without this the preserved dictionaries land in the file
        // with nothing naming them. Readers treat them as orphans and delete
        // them -- and with them the materials, visual styles and detail-view
        // styles that hang off them. On a corpus of real drawings the root
        // went from nineteen entries to one, every time, and the save said
        // nothing.
        //
        // An entry is only spliced when everything it reaches is re-emitted.
        // Naming a dictionary whose own children are missing is worse than
        // leaving it detached: ACAD_LAYOUT is in the raw net but LAYOUT
        // objects are not, so re-attaching it hands the reader a dictionary
        // pointing at handles that either belong to something else now or do
        // not exist, and at least one reader refuses the file outright.
        std::vector<std::pair<std::string, std::string>> rootEntries;
        std::size_t detached = 0;
        if (!_replay->rootDictEntries.empty()) {
            std::unordered_map<std::uint32_t, const DRW_RawDxfObject*> emitted;
            bool documentHasImages = false;
            const auto skippedImageHandles = skippedImageSourceHandles(documentHasImages, _replay.get());
            std::vector<DRW_RawDxfObject> plannedObjects;
            plannedObjects.reserve(_replay->objects.size());
            std::unordered_set<std::uint32_t> sanitizedDictionaries;
            for (const auto& object : _replay->objects) {
                if (object.handle != 0 && !skipsSourceImageObject(object, documentHasImages, _replay.get())) {
                    if (object.name == "DICTIONARY" && !skippedImageHandles.empty()) {
                        auto planned = object;
                        if (dropRegeneratedImageEntries(planned, skippedImageHandles)) {
                            sanitizedDictionaries.insert(planned.handle);
                            plannedObjects.push_back(std::move(planned));
                            emitted[object.handle] = &plannedObjects.back();
                            continue;
                        }
                    }
                    emitted[object.handle] = &object;
                }
            }

            // Mark missing targets, then propagate failure to every parent.
            // This visits shared edges once and handles cycles without caching
            // an optimistic result before the whole cycle has been checked.
            std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> parents;
            std::unordered_set<std::uint32_t> incomplete;
            std::vector<std::uint32_t> pending;
            const auto markIncomplete = [&](std::uint32_t handle) {
                if (incomplete.insert(handle).second) pending.push_back(handle);
            };
            for (const auto& item : emitted) {
                for (const auto& group : item.second->groups) {
                    // Ignore owners and reactors: they can point back to the
                    // root, which is regenerated rather than raw-replayed.
                    if (group.code() != 350 && group.code() != 360) continue;
                    if (group.type() != DRW_Variant::STRING || group.content.s == nullptr) continue;
                    std::uint32_t child = 0;
                    try {
                        child = static_cast<std::uint32_t>(std::stoul(*group.content.s, nullptr, 16));
                    } catch (const std::exception&) {
                        continue;
                    }
                    if (child == 0) continue;
                    const auto target = emitted.find(child);
                    if (target == emitted.end()) {
                        markIncomplete(item.first);
                        continue;
                    }
                    parents[child].push_back(item.first);
                    if (sanitizedDictionaries.count(item.first)) {
                        // After an entry is removed, reattach only the small
                        // independently checked XRECORD profile. Other child
                        // graphs remain detached until their owners and
                        // references can be checked.
                        if (!simpleTextXRecord(*target->second, item.first)) markIncomplete(item.first);
                    }
                }
            }
            // Any root reaching a filtered dictionary also needs a valid
            // ownership chain. Propagate once through shared paths and cycles.
            std::unordered_set<std::uint32_t> reachesSanitized = sanitizedDictionaries;
            std::vector<std::uint32_t> ownerCheck(sanitizedDictionaries.begin(), sanitizedDictionaries.end());
            for (std::size_t index = 0; index < ownerCheck.size(); ++index) {
                const auto child = ownerCheck[index];
                const auto found = parents.find(child);
                if (found == parents.end()) continue;
                for (const auto parent : found->second) {
                    if (emitted.at(child)->parentHandle != parent) markIncomplete(parent);
                    if (reachesSanitized.insert(parent).second) ownerCheck.push_back(parent);
                }
            }
            for (std::size_t index = 0; index < pending.size(); ++index) {
                const auto found = parents.find(pending[index]);
                if (found == parents.end()) continue;
                for (const auto parent : found->second) markIncomplete(parent);
            }

            for (const auto& entry : _replay->rootDictEntries) {
                const auto object = emitted.find(entry.second);
                const bool uncheckedRootOwner = reachesSanitized.count(entry.second) &&
                    (_replay->rootDictSourceAmbiguous || _replay->rootDictSourceHandle != kRootDictionaryHandle ||
                     object == emitted.end() ||
                     object->second->parentHandle != kRootDictionaryHandle);
                if ((documentHasImages && upperName(entry.first) == "ACAD_IMAGE_DICT") ||
                    object == emitted.end() || incomplete.count(entry.second) || uncheckedRootOwner) {
                    detached++;
                    continue;
                }
                const auto moved = remap.find(entry.second);
                const std::uint32_t handle =
                    moved == remap.end() ? entry.second : moved->second;
                rootEntries.emplace_back(entry.first, dxfW->toHexStrHandle(handle));
            }

            if (!rootEntries.empty()) {
                dxfW->setRootDictEntries(rootEntries);
            }
            if (detached > 0) {
                // Written, but with nothing naming them: a reader is entitled
                // to prune them. Saying so is the difference between a save
                // the user can act on and one that looks complete.
                _loss.droppedByType["unreferenced dictionaries"] += detached;
            }
        }

        dxfW->setDxfClasses(_replay->classes);
        dxfW->setRawDxfSections(_replay->sections);
    }

#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    if (!prepareDefaultLayoutGraph()) {
        _writeFailureReason = "The layout graph could not be safely prepared for this output; preserve the original and review the paper-space contents.";
        LOG_ERROR << "Cannot safely preserve the layout graph in " << filename;
        delete dxfW;
        dxfW = nullptr;
        return false;
    }
    const auto paperBlocks = _document->blocks();
    const bool primaryPaperHasContent = exportVersion >= DRW::AC1015 &&
        std::any_of(paperBlocks.begin(), paperBlocks.end(),
                    [this](const auto& block) {
                        return upperName(block->name()) == "*PAPER_SPACE" &&
                            !_document->entitiesByBlock(block).asVector().empty();
                    });
    if (_defaultLayouts.empty() && (primaryPaperHasContent || hasUnsafeAdditionalPaperSpace())) {
        _writeFailureReason = "The layout graph for paper-space contents or references cannot be retained in this output; preserve the original or use a supported ASCII DXF layout.";
        delete dxfW;
        dxfW = nullptr;
        return false;
    }
#endif
#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
    if (exportVersion > DRW::AC1009 && !prepareImageDefinitionPlan()) {
        _writeFailureReason = "The image definitions cannot be safely planned; preserve the original and review the image sources.";
        delete dxfW;
        dxfW = nullptr;
        return false;
    }
#endif
    const auto detached = _loss.droppedByType.find("unreferenced dictionaries");
    if (detached != _loss.droppedByType.end() && detached->second > 0)
        LOG_WARNING << filename << " carries " << detached->second
                    << " dictionary/dictionaries this build cannot re-attach to the root";
    if (_shelf && !_shelf->sourceImageDefinitionHandles.empty()) {
        std::unordered_set<std::uint32_t> retained;
        if (variantCarriesRecord(File::variantIdForType(_exportType), "IMAGE")) {
            visitDocumentImages([&](const lc::entity::Image_CSPtr& image) {
                const auto source = image->metaInfo<ImageDisplaySource>(ImageDisplaySource::key());
                if (source && source->sourceIdentity == _shelf->imageSourceIdentity && source->definition &&
                    validImageDefinitionSource(*source->definition, image->name()))
                    retained.insert(source->definitionHandle);
            }, true);
        }
        if (_replay) {
            bool hasImages = false;
            visitDocumentImages([&](const lc::entity::Image_CSPtr&) { hasImages = true; });
            for (const auto& object : _replay->objects)
                if (object.name == "IMAGEDEF" && !skipsSourceImageObject(object, hasImages, _replay.get()))
                    retained.insert(object.handle);
        }
        for (const auto handle : _shelf->sourceImageDefinitionHandles)
            if (!retained.count(handle)) ++_loss.droppedByType["IMAGEDEF"];
    }
    bool success = dxfW->write(this, exportVersion, isBinary);

    if (!success) {
        const auto diagnostic = dxfW->getLastDiagnostic();
        LOG_ERROR << "libdxfrw refused to write " << filename
                  << " (DRW::error " << dxfW->getError()
                  << ", " << diagnostic.code << ": " << diagnostic.message << ")";

        // Records this revision cannot carry no longer reach the library at
        // all -- writeEntity skips and counts them -- so a refusal here is
        // about the payload of one entity, which only libdxfrw can judge.
    }

    delete dxfW;
    dxfW = nullptr;

    return success;
}

void DXFimpl::writePoint(const lc::entity::Point_CSPtr& p) {
    DRW_Point point;
    getEntityAttributes(&point, p);
    // Issue #412 phase 2: propagate Z through — the lc::entity::Point IS a
    // geo::Coordinate, so it has a real z() value the writer used to drop.
    point.basePoint.x = p->x();
    point.basePoint.y = p->y();
    point.basePoint.z = p->z();
    dxfW->writePoint(&point);
}

void DXFimpl::writeLine(const lc::entity::Line_CSPtr& l) {
    DRW_Line line;
    getEntityAttributes(&line, l);
    // Issue #412 phase 2: preserve Z on both endpoints.
    line.basePoint.x = l->start().x();
    line.basePoint.y = l->start().y();
    line.basePoint.z = l->start().z();
    line.secPoint.x = l->end().x();
    line.secPoint.y = l->end().y();
    line.secPoint.z = l->end().z();
    dxfW->writeLine(&line);
}

void DXFimpl::writeCircle(const lc::entity::Circle_CSPtr& c) {
    DRW_Circle circle;
    getEntityAttributes(&circle, c);
    // Issue #412 phase 2: preserve Z of center.
    circle.basePoint.x = c->center().x();
    circle.basePoint.y = c->center().y();
    circle.basePoint.z = c->center().z();
    circle.radious = c->radius();
    dxfW->writeCircle(&circle);
}

void DXFimpl::writeArc(const lc::entity::Arc_CSPtr& a) {
    DRW_Arc arc;
    getEntityAttributes(&arc, a);
    // Issue #412 phase 2: preserve Z of center.
    arc.basePoint.x = a->center().x();
    arc.basePoint.y = a->center().y();
    arc.basePoint.z = a->center().z();
    arc.radious = a->radius();
    if (a->CCW()) {
        arc.staangle = a->startAngle();
        arc.endangle = a->endAngle();
    } else {
        arc.staangle = a->endAngle();
        arc.endangle = a->startAngle();
    }
    dxfW->writeArc(&arc);
}

void DXFimpl::writeEllipse(const lc::entity::Ellipse_CSPtr& s) {
    DRW_Ellipse el;
    getEntityAttributes(&el, s);
    // Issue #412 phase 2: preserve Z of both center and major axis point.
    el.basePoint.x = s->center().x();
    el.basePoint.y = s->center().y();
    el.basePoint.z = s->center().z();
    el.secPoint.x = s->majorP().x();
    el.secPoint.y = s->majorP().y();
    el.secPoint.z = s->majorP().z();
    el.ratio = 1/s->ratio();
    if (s->isReversed()) {
        el.staparam = s->endAngle();
        el.endparam = s->startAngle();
    } else {
        el.staparam = s->startAngle();
        el.endparam = s->endAngle();
    }
    dxfW->writeEllipse(&el);
}

void DXFimpl::writeSpline(const lc::entity::Spline_CSPtr& s) {
    DRW_Spline sp;

    getEntityAttributes(&sp, s);

    sp.knotslist = s->knotPoints();
    sp.weightlist = s->weights();
    sp.normalVec = DRW_Coord(s->nX(), s->nY(), s->nZ());
    sp.tgEnd = DRW_Coord(s->endTanX(), s->endTanY(), s->endTanZ());
    sp.tgStart = DRW_Coord(s->startTanX(), s->startTanY(), s->startTanZ());
    sp.degree = s->degree();

    for(const auto& cp : s->controlPoints()) {
        sp.controllist.push_back(std::make_shared<DRW_Coord>(cp.x(), cp.y(), cp.z()));
    }

    for(const auto& fp : s->fitPoints()) {
        sp.fitlist.push_back(std::make_shared<DRW_Coord>(fp.x(), fp.y(), fp.z()));
    }

    sp.flags = s->flags();
    sp.nknots = sp.knotslist.size();
    sp.nfit = sp.fitlist.size();
    sp.ncontrol = sp.controllist.size();

    dxfW->writeSpline(&sp);
}

void DXFimpl::writeInsert(const lc::entity::Insert_CSPtr& i) {
    DRW_Insert insert;
    getEntityAttributes(&insert, i);

    insert.name = i->displayBlock()->name();
    insert.basePoint.x = i->position().x();
    insert.basePoint.y = i->position().y();
    insert.basePoint.z = i->position().z();

    dxfW->writeInsert(&insert);
}

void DXFimpl::getEntityAttributes(DRW_Entity* ent, const lc::entity::CADEntity_CSPtr& entity) {
    auto layer_  = entity->layer();

    auto lpByValue = entity->metaInfo<lc::meta::DxfLinePatternByValue>(lc::meta::DxfLinePattern::LCMETANAME());
    auto lpByBlock = entity->metaInfo<lc::meta::DxfLinePatternByBlock>(lc::meta::DxfLinePattern::LCMETANAME());

    auto metaWidthByValue = entity->metaInfo<lc::meta::MetaLineWidthByValue>(lc::meta::MetaLineWidth::LCMETANAME());
    auto metaWidthByBlock = entity->metaInfo<lc::meta::MetaLineWidthByBlock>(lc::meta::MetaLineWidth::LCMETANAME());

    auto metaColorByBlock = entity->metaInfo<lc::meta::MetaColorByBlock>(lc::meta::MetaColor::LCMETANAME());
    auto metaColorByValue = entity->metaInfo<lc::meta::MetaColorByValue>(lc::meta::MetaColor::LCMETANAME());

    ent->layer = layer_->name();

    auto trueColor = entity->metaInfo<lc::meta::MetaColorTrueColor>(lc::meta::MetaColor::LCMETANAME());
    if (trueColor != nullptr) {
        lc::iColor col;
        if (_exportVersion > DRW::AC1015) {
            const auto color = trueColor->color();
            ent->color = trueColor->sourceAci();
            ent->color24 = (static_cast<int>(color.redI()) << 16)
                         | (static_cast<int>(color.greenI()) << 8)
                         | static_cast<int>(color.blueI());
            ent->colorName = trueColor->name();
        } else {
            ent->color = col.colorToInt(trueColor->color(), 1);
            const std::string note = "True colors were approximated using indexed colors for the selected DXF version.";
            if (std::find(_loss.notes.begin(), _loss.notes.end(), note) == _loss.notes.end())
                _loss.notes.push_back(note);
        }
    } else if(metaColorByBlock != nullptr) {
        ent->color = BYBLOCK_COLOR;
    }
    else if(metaColorByValue != nullptr) {
        lc::iColor col;
        auto color_ = col.colorToInt(metaColorByValue->color());
        ent->color = color_;
    }

    if(lpByValue != nullptr) {
        ent->lineType = lpByValue->name();
    }
    else if(lpByBlock != nullptr) {
        ent->lineType = LTYPE_BYBLOCK;
    }

    if (_exportVersion <= DRW::AC1014 && (metaWidthByValue || metaWidthByBlock)) {
        _loss.droppedByType["entity lineweight"]++;
    } else if(metaWidthByValue != nullptr) {
        ent->lWeight = static_cast<DRW_LW_Conv::lineWidth>(widthToInt(metaWidthByValue->width()));
    } else if(metaWidthByBlock != nullptr) {
        ent->lWeight = DRW_LW_Conv::lineWidth::widthByBlock;
    }
}

void DXFimpl::writeLTypes() {
    DRW_LType ltype;
    // Standard linetypes for LibreCAD / AutoCAD
    ltype.name = "CONTINUOUS";
    ltype.desc = "Solid line";
    dxfW->writeLineType(&ltype);
    // The reserved ByLayer and ByBlock rows have empty descriptions.
    ltype.desc.clear();
    ltype.name = "ByLayer";
    dxfW->writeLineType(&ltype);
    ltype.name = "ByBlock";
    dxfW->writeLineType(&ltype);

    ltype.name = "DOT";
    ltype.desc = "Dot . . . . . . . . . . . . . . . . . . . . . .";
    ltype.size = 2;
    ltype.length = 6.35;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DOTTINY";
    ltype.desc = "Dot (.15x) .....................................";
    ltype.size = 2;
    ltype.length = 0.9525;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DOT2";
    ltype.desc = "Dot (.5x) .....................................";
    ltype.size = 2;
    ltype.length = 3.175;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DOTX2";
    ltype.desc = "Dot (2x) .  .  .  .  .  .  .  .  .  .  .  .  .";
    ltype.size = 2;
    ltype.length = 12.7;
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHED";
    ltype.desc = "Dashed _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _";
    ltype.size = 2;
    ltype.length = 19.05;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHEDTINY";
    ltype.desc = "Dashed (.15x) _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _";
    ltype.size = 2;
    ltype.length = 2.8575;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHED2";
    ltype.desc = "Dashed (.5x) _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _";
    ltype.size = 2;
    ltype.length = 9.525;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHEDX2";
    ltype.desc = "Dashed (2x) ____  ____  ____  ____  ____  ___";
    ltype.size = 2;
    ltype.length = 38.1;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOT";
    ltype.desc = "Dash dot __ . __ . __ . __ . __ . __ . __ . __";
    ltype.size = 4;
    ltype.length = 25.4;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOTTINY";
    ltype.desc = "Dash dot (.15x) _._._._._._._._._._._._._._._.";
    ltype.size = 4;
    ltype.length = 3.81;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOT2";
    ltype.desc = "Dash dot (.5x) _._._._._._._._._._._._._._._.";
    ltype.size = 4;
    ltype.length = 12.7;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DASHDOTX2";
    ltype.desc = "Dash dot (2x) ____  .  ____  .  ____  .  ___";
    ltype.size = 4;
    ltype.length = 50.8;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDE";
    ltype.desc = "Divide ____ . . ____ . . ____ . . ____ . . ____";
    ltype.size = 6;
    ltype.length = 31.75;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDETINY";
    ltype.desc = "Divide (.15x) __..__..__..__..__..__..__..__.._";
    ltype.size = 6;
    ltype.length = 4.7625;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDE2";
    ltype.desc = "Divide (.5x) __..__..__..__..__..__..__..__.._";
    ltype.size = 6;
    ltype.length = 15.875;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "DIVIDEX2";
    ltype.desc = "Divide (2x) ________  .  .  ________  .  .  _";
    ltype.size = 6;
    ltype.length = 63.5;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDER";
    ltype.desc = "Border __ __ . __ __ . __ __ . __ __ . __ __ .";
    ltype.size = 6;
    ltype.length = 44.45;
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(12.7);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDERTINY";
    ltype.desc = "Border (.15x) __.__.__.__.__.__.__.__.__.__.__.";
    ltype.size = 6;
    ltype.length = 6.6675;
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(1.905);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDER2";
    ltype.desc = "Border (.5x) __.__.__.__.__.__.__.__.__.__.__.";
    ltype.size = 6;
    ltype.length = 22.225;
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(6.35);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "BORDERX2";
    ltype.desc = "Border (2x) ____  ____  .  ____  ____  .  ___";
    ltype.size = 6;
    ltype.length = 88.9;
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(25.4);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(0.0);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTER";
    ltype.desc = "Center ____ _ ____ _ ____ _ ____ _ ____ _ ____";
    ltype.size = 4;
    ltype.length = 50.8;
    ltype.path.push_back(31.75);
    ltype.path.push_back(-6.35);
    ltype.path.push_back(6.35);
    ltype.path.push_back(-6.35);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTERTINY";
    ltype.desc = "Center (.15x) ___ _ ___ _ ___ _ ___ _ ___ _ ___";
    ltype.size = 4;
    ltype.length = 7.62;
    ltype.path.push_back(4.7625);
    ltype.path.push_back(-0.9525);
    ltype.path.push_back(0.9525);
    ltype.path.push_back(-0.9525);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTER2";
    ltype.desc = "Center (.5x) ___ _ ___ _ ___ _ ___ _ ___ _ ___";
    ltype.size = 4;
    ltype.length = 28.575;
    ltype.path.push_back(19.05);
    ltype.path.push_back(-3.175);
    ltype.path.push_back(3.175);
    ltype.path.push_back(-3.175);
    dxfW->writeLineType(&ltype);

    ltype.path.clear();
    ltype.name = "CENTERX2";
    ltype.desc = "Center (2x) ________  __  ________  __  _____";
    ltype.size = 4;
    ltype.length = 101.6;
    ltype.path.push_back(63.5);
    ltype.path.push_back(-12.7);
    ltype.path.push_back(12.7);
    ltype.path.push_back(-12.7);
    dxfW->writeLineType(&ltype);

    // Issue #412 phase 3: emit every DxfLinePatternByValue registered on the
    // document, not just the hardcoded set above.  Without this, entities
    // referencing a user-imported LTYPE (very common when a supplier's DXF
    // is opened and re-saved) point at an absent LTYPE table entry — the
    // resulting file is structurally invalid DXF.  Skip names already
    // covered by the hardcoded standard set to avoid duplicate table entries.
    static const std::set<std::string> kBuiltin{
        "CONTINUOUS", "ByLayer", "ByBlock",
        "DOT", "DOTTINY", "DOT2", "DOTX2",
        "DASHED", "DASHEDTINY", "DASHED2", "DASHEDX2",
        "DASHDOT", "DASHDOTTINY", "DASHDOT2", "DASHDOTX2",
        "DIVIDE", "DIVIDETINY", "DIVIDE2", "DIVIDEX2",
        "BORDER", "BORDERTINY", "BORDER2", "BORDERX2",
        "CENTER", "CENTERTINY", "CENTER2", "CENTERX2"};
    for (const auto& lp : _document->linePatterns()) {
        if (lp == nullptr) continue;
        const std::string& name = lp->name();
        if (name.empty() || kBuiltin.count(name)) continue;

        DRW_LType user;
        user.name = name;
        user.desc = lp->description();
        user.path = lp->path();
        user.size = static_cast<int>(user.path.size());
        user.length = lp->length();
        dxfW->writeLineType(&user);
    }
}

void DXFimpl::writeAppId() {
    DRW_AppId ai;
    ai.name ="LibreCad";
    dxfW->writeAppId(&ai);
#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
    for (const auto& entry : _imageXdataAppIds) {
        if (entry.first == "ACAD" || entry.first == "LIBRECAD") continue;
        DRW_AppId extra;
        extra.name = entry.second;
        dxfW->writeAppId(&extra);
    }
#endif
}

// Issue #412 phase 3: writeHeader was defaulted to empty; drawings saved by
// LibreCAD carried no HEADER variables at all.  Some readers (AutoCAD in
// strict mode, ezdxf pedantic mode) reject a file with no $ACADVER, and
// downstream tools that key off $INSUNITS silently pick the wrong unit.
// Emit a small viable set — the version tag matches whatever writeDXF
// selected, the units default to Millimeter, measurement to Metric.
void DXFimpl::writeHeader(DRW_Header& data) {
    // Put back what the drawing said, if it said anything. lckernel has nowhere
    // to keep units, so without this a drawing in inches is read and written
    // back as millimetres -- and the next person to open it sees it rescaled,
    // with nothing to indicate that LibreCAD did it.
    const auto preserved = preservedOn(_document);
    if (preserved != nullptr && preserved->hasHeader) {
        if (preserved->hasMeasurement) {
            if (_exportVersion > DRW::AC1009) data.addInt("$MEASUREMENT", preserved->measurement, 70);
            else _loss.droppedByType["$MEASUREMENT"]++;
        }
        if (preserved->hasInsUnits) {
            if (_exportVersion > DRW::AC1014) data.addInt("$INSUNITS", preserved->insUnitsCode, 70);
            else _loss.droppedByType["$INSUNITS"]++;
        }
        if (preserved->lineTypeScale > 0.0) {
            data.addDouble("$LTSCALE", preserved->lineTypeScale, 40);
        }
        return;
    }

    // No $ACADVER here. DRW_Header::write() emits it from the version the
    // write is actually targeted at, and then consumes whatever we stored
    // under that key and discards it (src/drw_header.cpp:134-167) -- so a
    // value set here could never reach the file, and the "AC1015" that used to
    // sit here could only mislead the next reader of this function.
    // Default to Metric with millimeter insertion units.  The document API
    // doesn't currently expose a unit-of-measure field for the whole
    // drawing, so hard-code the sensible default rather than the DXF
    // spec-default of 0 (unitless).
    if (_exportVersion > DRW::AC1009) data.addInt("$MEASUREMENT", 1, 70); // 0 = English, 1 = Metric
    if (_exportVersion > DRW::AC1014) data.addInt("$INSUNITS", 4, 70);    // 4 = Millimeter
}

// Issue #412 phase 1: populate the DRW_Dimension base header shared by all
// five subtypes.  Kept out of the per-subtype writers because getEntityAttributes
// needs the CADEntity pointer (which the Dimension base doesn't carry).
void DXFimpl::writeDimensionCommon(DRW_Dimension* dim,
                                   const lc::entity::CADEntity_CSPtr& entity,
                                   const lc::entity::Dimension& d) {
    getEntityAttributes(dim, entity);

    dim->setDefPoint(DRW_Coord(d.definitionPoint().x(),
                               d.definitionPoint().y(),
                               d.definitionPoint().z()));
    dim->setTextPoint(DRW_Coord(d.middleOfText().x(),
                                d.middleOfText().y(),
                                d.middleOfText().z()));
    dim->setAlign(static_cast<int>(d.attachmentPoint()));
    dim->setDir(d.textAngle());
    dim->setTextLineFactor(d.lineSpacingFactor());
    dim->setTextLineStyle(static_cast<int>(d.lineSpacingStyle()));
    dim->setText(d.explicitValue());
    // Style must be non-empty or the R2000+ writer path skips writing group
    // 3 entirely, then downstream readers reject the dimension header.
    dim->setStyle("STANDARD");

    // The block holding this dimension's drawn geometry, written by
    // writeBlocks() before any entity is emitted. Without group 2 an auditing
    // reader does not regenerate the picture -- it deletes the dimension.
    const auto block = _dimensionBlocks.find(entity->id());
    if (block != _dimensionBlocks.end()) {
        dim->setName(block->second);
    }
}

void DXFimpl::writeDimLinear(const lc::entity::DimLinear_CSPtr& d) {
    DRW_DimLinear dim;
    writeDimensionCommon(&dim, d, *d);
    // DXF code 70 subtype: 0 = linear (rotated/horizontal/vertical).  Without
    // setting this, libdxfrw's reader would treat every dim as linear anyway,
    // but the writer's dispatch chain still keys off eType (set in ctor).
    dim.type = 0;
    dim.setDef1Point(DRW_Coord(d->definitionPoint2().x(),
                               d->definitionPoint2().y(),
                               d->definitionPoint2().z()));
    dim.setDef2Point(DRW_Coord(d->definitionPoint3().x(),
                               d->definitionPoint3().y(),
                               d->definitionPoint3().z()));
    dim.setAngle(d->angle() * 180 / M_PI);
    dim.setOblique(d->oblique() * 180 / M_PI);
    dxfW->writeDimension(&dim);
}

void DXFimpl::writeDimAligned(const lc::entity::DimAligned_CSPtr& d) {
    DRW_DimAligned dim;
    writeDimensionCommon(&dim, d, *d);
    dim.type = 1;  // aligned
    dim.setDef1Point(DRW_Coord(d->definitionPoint2().x(),
                               d->definitionPoint2().y(),
                               d->definitionPoint2().z()));
    dim.setDef2Point(DRW_Coord(d->definitionPoint3().x(),
                               d->definitionPoint3().y(),
                               d->definitionPoint3().z()));
    dxfW->writeDimension(&dim);
}

void DXFimpl::writeDimRadial(const lc::entity::DimRadial_CSPtr& d) {
    DRW_DimRadial dim;
    writeDimensionCommon(&dim, d, *d);
    dim.type = 4;  // radius
    // setDefPoint was called with definitionPoint() in the common helper.
    // For radial dims, defPoint IS the center (see addDimRadial reader).
    dim.setDiameterPoint(DRW_Coord(d->definitionPoint2().x(),
                                   d->definitionPoint2().y(),
                                   d->definitionPoint2().z()));
    dim.setLeaderLength(d->leader());
    dxfW->writeDimension(&dim);
}

void DXFimpl::writeDimDiametric(const lc::entity::DimDiametric_CSPtr& d) {
    DRW_DimDiametric dim;
    writeDimensionCommon(&dim, d, *d);
    dim.type = 3;  // diameter
    // For diametric dims the reader treats defPoint (10) as diameter2Point
    // and getPt5/code 15 as diameter1Point.  The lc entity's
    // definitionPoint()  is diameter1 (that's how addDimDiametric reads it),
    // definitionPoint2()  is diameter2.  Match that pairing exactly.
    dim.setDiameter1Point(DRW_Coord(d->definitionPoint().x(),
                                    d->definitionPoint().y(),
                                    d->definitionPoint().z()));
    dim.setDiameter2Point(DRW_Coord(d->definitionPoint2().x(),
                                    d->definitionPoint2().y(),
                                    d->definitionPoint2().z()));
    dim.setLeaderLength(d->leader());
    dxfW->writeDimension(&dim);
}

void DXFimpl::writeDimAngular(const lc::entity::DimAngular_CSPtr& d) {
    DRW_DimAngular dim;
    writeDimensionCommon(&dim, d, *d);
    dim.type = 2;  // angular (2-line)
    dim.setFirstLine1(DRW_Coord(d->defLine11().x(),
                                d->defLine11().y(),
                                d->defLine11().z()));
    dim.setFirstLine2(DRW_Coord(d->defLine12().x(),
                                d->defLine12().y(),
                                d->defLine12().z()));
    dim.setSecondLine1(DRW_Coord(d->defLine21().x(),
                                 d->defLine21().y(),
                                 d->defLine21().z()));
    dim.setSecondLine2(DRW_Coord(d->defLine22().x(),
                                 d->defLine22().y(),
                                 d->defLine22().z()));
    // Angular's DimPoint (code 16, arcPoint) is used for arc placement;
    // reuse definitionPoint() so the exported file has a plausible value.
    dim.setDimPoint(DRW_Coord(d->definitionPoint().x(),
                              d->definitionPoint().y(),
                              d->definitionPoint().z()));
    dxfW->writeDimension(&dim);
}

void DXFimpl::writeDimOrdinate(const lc::entity::DimOrdinate_CSPtr& d) {
    DRW_DimOrdinate dim;
    writeDimensionCommon(&dim, d, *d);
    // Bit 6 of group 70 is the axis: set, the ordinate measures X. The
    // subtype, 6, is added by writeDimension, which keeps the high bits.
    dim.type = d->xType() ? 64 : 0;
    dim.setFirstLine(DRW_Coord(d->featurePoint().x(),
                               d->featurePoint().y(),
                               d->featurePoint().z()));
    dim.setSecondLine(DRW_Coord(d->leaderEndPoint().x(),
                                d->leaderEndPoint().y(),
                                d->leaderEndPoint().z()));
    dxfW->writeDimension(&dim);
}

// Issue #412 phase 1: dispatched, but body was empty — LWPolylines vanished
// from every save.  Mirror addLWPolyline's field mapping.
void DXFimpl::writeLWPolyline(const lc::entity::LWPolyline_CSPtr& p) {
    // LWPOLYLINE arrived with R13. Asking libdxfrw to write one at R12 is not a
    // degraded polyline, it is a refused write -- and a refused write loses the
    // whole file. R12's POLYLINE carries the same geometry, vertex widths and
    // bulges included, and DXFimpl::addPolyline already reads one back into an
    // LWPolyline, so the down-convert is a round trip rather than a loss.
    if (_exportVersion <= DRW::AC1009) {
        DRW_Polyline pl;
        getEntityAttributes(&pl, p);

        pl.thickness = p->tickness();
        pl.basePoint.z = p->elevation();
        pl.extPoint.x = p->extrusionDirection().x();
        pl.extPoint.y = p->extrusionDirection().y();
        pl.extPoint.z = p->extrusionDirection().z();
        pl.flags = p->closed() ? 1 : 0;
        pl.defstawidth = p->width();
        pl.defendwidth = p->width();

        for (const auto& v : p->vertex()) {
            DRW_Vertex vertex(v.location().x(), v.location().y(), p->elevation(), v.bulge());
            vertex.stawidth = v.startWidth();
            vertex.endwidth = v.endWidth();
            pl.addVertex(vertex);
        }
        pl.vertexcount = pl.vertlist.size();

        if (!dxfW->writePolyline(&pl)) {
            LOG_ERROR << "libdxfrw refused POLYLINE";
        }
        return;
    }

    DRW_LWPolyline pl;
    getEntityAttributes(&pl, p);

    pl.width = p->width();
    pl.elevation = p->elevation();
    pl.thickness = p->tickness();
    pl.extPoint.x = p->extrusionDirection().x();
    pl.extPoint.y = p->extrusionDirection().y();
    pl.extPoint.z = p->extrusionDirection().z();
    // DXF group 70 bit 0 = "closed"; addLWPolyline reads it back the same way.
    pl.flags = p->closed() ? 1 : 0;

    for (const auto& v : p->vertex()) {
        auto vert = std::make_shared<DRW_Vertex2D>();
        vert->x = v.location().x();
        vert->y = v.location().y();
        vert->stawidth = v.startWidth();
        vert->endwidth = v.endWidth();
        vert->bulge = v.bulge();
        pl.vertlist.push_back(vert);
    }
    pl.vertexnum = pl.vertlist.size();

    dxfW->writeLWPolyline(&pl);
}

// Issue #412 phase 1: dispatched, but body was empty — images vanished.
// Matches libdxfrw's writeImage(ent, name) contract; name is used to key the
// ImageDef object dictionary.
void DXFimpl::writeImage(const lc::entity::Image_CSPtr& i) {
    DRW_Image img;
    getEntityAttributes(&img, i);

    img.basePoint.x = i->base().x();
    img.basePoint.y = i->base().y();
    img.basePoint.z = i->base().z();
    // secPoint holds the U-vector (per single pixel), vVector the V-vector;
    // sizeu/sizev are the image size in pixels — mapped to width/height here
    // because that's what the reader (linkImage) rebuilds the Image from.
    img.secPoint.x = i->uv().x();
    img.secPoint.y = i->uv().y();
    img.secPoint.z = i->uv().z();
    img.vVector.x = i->vv().x();
    img.vVector.y = i->vv().y();
    img.vVector.z = i->vv().z();
    img.sizeu = i->width();
    img.sizev = i->height();
    img.brightness = static_cast<int>(i->brightness());
    img.contrast   = static_cast<int>(i->contrast());
    img.fade       = static_cast<int>(i->fade());

    const auto source = i->metaInfo<ImageDisplaySource>(ImageDisplaySource::key());
    if (source) {
        img.m_classVersion = source->classVersion;
        img.m_displayProps = source->displayProps;
        if (_exportVersion >= DRW::AC1024) {
            img.clipMode = source->clipMode;
        } else if (source->clipMode) {
            ++_loss.droppedByType["IMAGE clip mode"];
        }
        if (!source->boundaryDropped && source->width == img.sizeu && source->height == img.sizev) {
            img.clip = source->clip;
            img.m_clipBoundaryType = source->boundaryType;
            img.clipPath = source->boundary;
        } else {
            ++_loss.droppedByType["IMAGE clipping"];
        }
    }

    DRW_ImageDef* definition = nullptr;
    bool usedSourcePlan = false;
    bool droppedPlannedXdata = false;
#ifdef LIBDXFRW_HAS_DXF_STAGED_IMAGE_DEFINITION_SCALAR_INSPECTOR
    const auto generation = source ? _imageSourceGenerations.find(source->sourceIdentity) : _imageSourceGenerations.end();
    if (source && source->definition && validImageDefinitionSource(*source->definition, i->name()) &&
        generation != _imageSourceGenerations.end() &&
        _plannedImageDefinitions.count({generation->second, source->definitionHandle})) {
        definition = dxfW->writeImage(&img, i->name(), generation->second, source->definitionHandle);
        usedSourcePlan = definition != nullptr;
        droppedPlannedXdata = _imageXdataPartialSources.count({generation->second, source->definitionHandle}) != 0;
    }
    else
#endif
        definition = dxfW->writeImage(&img, i->name());
    if (!definition) return;
    const ImageDefinitionOutputBinding binding{source ? source->sourceIdentity : nullptr,
                                               source ? source->definitionHandle : 0};
    const auto inserted = _emittedImageDefinitions.emplace(definition, binding);
    if (!inserted.second) {
        if (source && (inserted.first->second.sourceIdentity != binding.sourceIdentity ||
                       inserted.first->second.sourceHandle != binding.sourceHandle)) {
            ++_loss.droppedByType["IMAGEDEF identity"];
            if (source->definition && (source->definition->unmodeledMetadata || droppedPlannedXdata ||
                                       (!usedSourcePlan && (!source->definition->appData.empty() || !source->definition->xdata.empty()))))
                ++_loss.droppedByType["IMAGEDEF metadata"];
        }
        return;
    }
    const auto original = source ? source->definition : nullptr;
    if (original && (original->unmodeledMetadata || droppedPlannedXdata || (!usedSourcePlan && (!original->appData.empty() || !original->xdata.empty()))))
        ++_loss.droppedByType["IMAGEDEF metadata"];
    if (original && validImageDefinitionSource(*original, i->name())) {
        definition->imgVersion = original->version;
        definition->u = original->u; definition->v = original->v;
        definition->up = original->up; definition->vp = original->vp;
        definition->loaded = original->loaded; definition->resolution = original->resolution;
    } else {
        // A newly authored IMAGE has no source definition to replay.
        definition->u = img.sizeu; definition->v = img.sizev;
        definition->up = definition->vp = 1;
        definition->loaded = 1; definition->resolution = 0;
        if (source) ++_loss.droppedByType["IMAGEDEF fields"];
    }
}

// Issue #412 phase 1: written from scratch — the reader ingests HATCH but
// no writer existed, not even a declaration.  Only Line, Arc, Ellipse edge
// types are emitted here; libdxfrw's writeHatch itself has no support for
// SPLINE or POLYLINE boundary paths (code marked `//RLZ: TODO`), so trying
// to emit them would just produce silently-ignored bytes.
namespace {

// One DXF boundary edge for a kernel entity, or null when the kind has no edge
// representation. Boundary edges carry geometry only: no layer, colour or
// line type, which is why these are built bare rather than through
// getEntityAttributes().
std::shared_ptr<DRW_Entity> hatchEdge(const lc::entity::CADEntity_CSPtr& entity) {
    if (auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
        auto edge = std::make_shared<DRW_Line>();
        edge->basePoint.x = line->start().x();
        edge->basePoint.y = line->start().y();
        edge->secPoint.x = line->end().x();
        edge->secPoint.y = line->end().y();
        return edge;
    }

    if (auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
        auto edge = std::make_shared<DRW_Arc>();
        edge->basePoint.x = arc->center().x();
        edge->basePoint.y = arc->center().y();
        edge->radious = arc->radius();
        edge->staangle = arc->startAngle();
        edge->endangle = arc->endAngle();
        edge->isccw = arc->CCW() ? 1 : 0;
        return edge;
    }

    if (auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity)) {
        auto edge = std::make_shared<DRW_Ellipse>();
        edge->basePoint.x = ellipse->center().x();
        edge->basePoint.y = ellipse->center().y();
        edge->secPoint.x = ellipse->majorP().x();
        edge->secPoint.y = ellipse->majorP().y();
        edge->ratio = 1.0 / ellipse->ratio();
        edge->staparam = ellipse->startAngle();
        edge->endparam = ellipse->endAngle();
        edge->isccw = ellipse->isReversed() ? 0 : 1;
        return edge;
    }

    if (auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
        auto edge = std::make_shared<DRW_Spline>();
        edge->degree = spline->degree();
        edge->flags = spline->flags();
        if (!spline->weights().empty()) edge->flags |= lc::geo::Spline::RATIONAL;
        edge->knotslist = spline->knotPoints();
        edge->weightlist = spline->weights();
        edge->tgStart = DRW_Coord(spline->startTanX(), spline->startTanY(), spline->startTanZ());
        edge->tgEnd = DRW_Coord(spline->endTanX(), spline->endTanY(), spline->endTanZ());

        for (const auto& cp : spline->controlPoints()) {
            edge->controllist.push_back(std::make_shared<DRW_Coord>(cp.x(), cp.y(), cp.z()));
        }
        for (const auto& fp : spline->fitPoints()) {
            edge->fitlist.push_back(std::make_shared<DRW_Coord>(fp.x(), fp.y(), fp.z()));
        }

        edge->nknots = edge->knotslist.size();
        edge->ncontrol = edge->controllist.size();
        edge->nfit = edge->fitlist.size();
        return edge;
    }

    return nullptr;
}

/** The polyline boundary for a loop that is one LWPolyline and nothing else. */
std::shared_ptr<DRW_LWPolyline> hatchPolyline(const lc::entity::LWPolyline_CSPtr& polyline) {
    auto boundary = std::make_shared<DRW_LWPolyline>();
    // DXF group 70 bit 0 = closed, as writeLWPolyline and addHatch both read it.
    boundary->flags = polyline->closed() ? 1 : 0;
    boundary->elevation = polyline->elevation();
    boundary->thickness = polyline->tickness();
    boundary->width = polyline->width();

    for (const auto& vertex : polyline->vertex()) {
        auto vert = std::make_shared<DRW_Vertex2D>();
        vert->x = vertex.location().x();
        vert->y = vertex.location().y();
        vert->bulge = vertex.bulge();
        vert->stawidth = vertex.startWidth();
        vert->endwidth = vertex.endWidth();
        boundary->vertlist.push_back(vert);
    }
    boundary->vertexnum = boundary->vertlist.size();

    return boundary;
}

/**
 * Add one boundary entity to a loop as edges.
 *
 * A polyline is decomposed into the segments it draws, because DXF only allows
 * one in a loop of its own -- see writeHatch.
 */
void appendHatchEdges(DRW_HatchLoop& loop, const lc::entity::CADEntity_CSPtr& entity) {
    if (auto polyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity)) {
        for (const auto& segment : polyline->asEntities()) {
            if (auto edge = hatchEdge(segment)) {
                loop.objlist.push_back(edge);
            }
        }
        return;
    }

    if (auto edge = hatchEdge(entity)) {
        loop.objlist.push_back(edge);
        return;
    }

    LOG_WARNING << "Dropping a hatch boundary edge with no DXF representation";
}

}  // namespace

void DXFimpl::writeHatch(const lc::entity::Hatch_CSPtr& h) {
    DRW_Hatch hatch;
    getEntityAttributes(&hatch, h);

    hatch.name = h->getPatternName();
    hatch.solid = h->isSolid() ? 1 : 0;
    hatch.associative = 0;
    hatch.hstyle = 0;
    hatch.hpattern = 1;
    hatch.doubleflag = 0;
    hatch.angle = h->getAngle();
    hatch.scale = h->getScale();
    hatch.deflines = 0;

    const auto& region = h->getRegion();
    for (const auto& loop : region.loopList()) {
        const auto& entities = loop.entities();
        for (const auto& entity : entities) {
            const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(entity);
            if (spline && !spline->weights().empty()
                && spline->weights().size() != spline->controlPoints().size()) {
                LOG_ERROR << "Dropping HATCH with an incomplete spline weight vector";
                _loss.droppedByType["HATCH"]++;
                return;
            }
        }

        // DXF says a polyline boundary path is the whole path: group 92 bit 1
        // means "this loop is one polyline", and libdxfrw enforces it --
        // dxfRW::writeHatch refuses a hatch whose bit-1 loop does not hold
        // exactly one DRW_LWPolyline, and a refused write loses the entire
        // file, not just the hatch. So the bit is set only for a loop that is
        // one polyline; anywhere else the polyline goes in as its segments.
        lc::entity::LWPolyline_CSPtr wholeLoopPolyline;
        if (entities.size() == 1) {
            wholeLoopPolyline =
                std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entities.front());
        }

        auto drwLoop = std::make_shared<DRW_HatchLoop>(wholeLoopPolyline ? 2 : 0);
        if (wholeLoopPolyline) {
            drwLoop->objlist.push_back(hatchPolyline(wholeLoopPolyline));
        } else {
            for (const auto& entity : entities) {
                appendHatchEdges(*drwLoop, entity);
            }
        }

        // A loop with no edges is not a boundary: it reloads as an empty
        // geo::Loop, which is what the kernel guards had to be taught to
        // survive. Dropping it keeps the rest of the hatch.
        if (drwLoop->objlist.empty()) {
            LOG_WARNING << "Dropping a hatch boundary loop that produced no edges";
            _loss.droppedByType["HATCH boundary loop"]++;
            continue;
        }

        drwLoop->update();
        hatch.appendLoop(drwLoop);
    }

    if (hatch.looplist.empty()) {
        // Counted, not just logged. Every other record this writer leaves out
        // reaches the user through the export result, and a hatch that vanishes
        // from a saved drawing is not less worth saying.
        LOG_WARNING << "Dropping HATCH " << hatch.name << ": no boundary survived";
        _loss.droppedByType["HATCH"]++;
        return;
    }
    hatch.loopsnum = hatch.looplist.size();

    const auto snapshot = h->metaInfo<MPolygonSnapshot>(MPolygonSnapshot::key());
    double outputOffsetX = 0.0, outputOffsetY = 0.0;
    double outputElevation = 0.0;
    bool rewriteBoundary = false;
    if (snapshot && _exportVersion > DRW::AC1009 && snapshot->canWrite(_exportVersion)
        && snapshot->replayable(hatch, h, outputOffsetX, outputOffsetY, outputElevation, rewriteBoundary)) {
        DRW_MPolygon polygon(*snapshot->source);
        // MPOLYGON stores vertices relative to its offset. A planar translation
        // changes the offset, leaving the serialized boundary coordinates intact.
        polygon.xDirX = outputOffsetX;
        polygon.xDirY = outputOffsetY;
        polygon.basePoint.z = outputElevation;
        if (rewriteBoundary) {
            auto* saved = static_cast<DRW_LWPolyline*>(polygon.looplist[0]->objlist[0].get());
            const auto* current = static_cast<const DRW_LWPolyline*>(hatch.looplist[0]->objlist[0].get());
            for (std::size_t i = 0; i < saved->vertlist.size(); ++i) {
                saved->vertlist[i]->x = current->vertlist[i]->x - outputOffsetX;
                saved->vertlist[i]->y = current->vertlist[i]->y - outputOffsetY;
            }
        }
        // A new document assigns new entity and owner handles; the qualified
        // profile has no other retained reference edges to remap.
        polygon.handle = DRW::NoHandle;
        polygon.parentHandle = DRW::NoHandle;
        if (!dxfW->writeMPolygon(&polygon)) {
            LOG_ERROR << "libdxfrw refused MPOLYGON " << hatch.name;
            _loss.droppedByType["MPOLYGON"]++;
        }
        return;
    }
    if (snapshot && _mpolygonsWrittenAsHatch++ == 0) {
        _loss.notes.push_back(
            "Some MPOLYGON entities were written as HATCH; boundary geometry "
            "or polygon-specific information may not be retained.");
    }
    if (snapshot && hatch.solid) {
        const auto fill = h->metaInfo<lc::meta::MetaFillColor>(lc::meta::MetaFillColor::LCMETANAME());
        if (fill) {
            const int sourceAci = fill->sourceAci();
            const int fillAci = sourceAci > 0 && sourceAci < 256
                ? sourceAci : icol.colorToInt(fill->color());
            if (hatch.color != fillAci || hatch.color24 >= 0 || !hatch.colorName.empty()) {
                const std::string note = "A projected HATCH uses the MPOLYGON fill color; its separate boundary color cannot be retained.";
                if (std::find(_loss.notes.begin(), _loss.notes.end(), note) == _loss.notes.end())
                    _loss.notes.push_back(note);
            }
            hatch.color = fillAci;
            hatch.color24 = -1;
            hatch.colorName.clear();
        }
    }
    // Supported paths can share one default-normal HATCH elevation. Do not
    // infer it from unsupported or mixed loops.
    bool planar = !region.loopList().empty();
    bool haveElevation = false;
    bool mixedOrNonfiniteElevation = false;
    double elevation = 0.0;
    const auto addElevation = [&](double z) {
        if (!std::isfinite(z) || (haveElevation && z != elevation)) {
            mixedOrNonfiniteElevation = true;
            return false;
        }
        elevation = z;
        haveElevation = true;
        return true;
    };
    for (const auto& loop : region.loopList()) {
        const auto& entities = loop.entities();
        if (entities.empty()) { planar = false; break; }
        for (const auto& entity : entities) {
            const auto polyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity);
            if (polyline) {
                if (entities.size() != 1 || !polyline->closed() || polyline->vertex().empty()
                    || polyline->elevation() != 0.0 || polyline->tickness() != 0.0
                    || polyline->extrusionDirection().x() != 0.0
                    || polyline->extrusionDirection().y() != 0.0
                    || polyline->extrusionDirection().z() != 1.0) {
                    planar = false; break;
                }
                for (const auto& vertex : polyline->vertex())
                    if (!addElevation(vertex.location().z())) { planar = false; break; }
            } else if (const auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
                if (!addElevation(line->start().z()) || !addElevation(line->end().z()))
                    planar = false;
            } else if (const auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
                if (!addElevation(arc->center().z())) planar = false;
            } else if (const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity)) {
                if (ellipse->majorP().z() != 0.0 || !addElevation(ellipse->center().z()))
                    planar = false;
            } else if (const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
                if (spline->controlPoints().empty()
                    || spline->startTanZ() != 0.0 || spline->endTanZ() != 0.0) {
                    planar = false; break;
                }
                for (const auto& point : spline->controlPoints())
                    if (!addElevation(point.z())) { planar = false; break; }
                if (planar) for (const auto& point : spline->fitPoints())
                    if (!addElevation(point.z())) { planar = false; break; }
            } else {
                planar = false;
            }
            if (!planar) break;
        }
        if (!planar) break;
    }
    if (planar && haveElevation) {
        hatch.basePoint.z = elevation;
        // Spline edge points are 2D in DXF. The common elevation above is
        // their Z component, so do not submit a second geometric Z to the
        // codec's strict HATCH boundary validator.
        for (const auto& loop : hatch.looplist) {
            for (const auto& entity : loop->objlist) {
                const auto spline = std::dynamic_pointer_cast<DRW_Spline>(entity);
                if (!spline) continue;
                for (const auto& point : spline->controllist) if (point) point->z = 0.0;
                for (const auto& point : spline->fitlist) if (point) point->z = 0.0;
            }
        }
    }
    if (mixedOrNonfiniteElevation && !snapshot) {
        _loss.notes.push_back(
            "HATCH boundary elevation varies or is nonfinite; the output "
            "cannot preserve its three-dimensional boundary.");
    }
    if (!dxfW->writeHatch(&hatch)) {
        LOG_ERROR << "libdxfrw refused HATCH " << hatch.name;
        _loss.droppedByType["HATCH"]++;
    }
}

void DXFimpl::writeText(const lc::entity::Text_CSPtr& t) {
    DRW_Text tex;
    getEntityAttributes(&tex, t);

    // A TEXT record is one line and has no way to say otherwise. The line
    // this replaces substituted a lone backslash for each newline, which
    // escapes nothing and leaves a stray `\` in the drawing; a space at least
    // reads as the word break the newline stood for. A drawing that needs real
    // line breaks needs an MTEXT.
    std::string correctedText = t->text_value();
    std::replace(correctedText.begin(), correctedText.end(), '\n', ' ');

    // Issue #412 phase 2: preserve Z on insertion point.
    tex.basePoint.x = t->insertion_point().x();
    tex.basePoint.y = t->insertion_point().y();
    tex.basePoint.z = t->insertion_point().z();
    tex.text = correctedText;
    tex.textgen = t->textgeneration();
    tex.height = t->height();
    tex.angle = t->angle() * 180 / M_PI;
    tex.alignH = DRW_Text::HAlign(t->halign());
    tex.alignV = DRW_Text::VAlign(t->valign());
    // Issue #412 phase 2: text style was never written; downstream readers
    // would fall back to "STANDARD" but any user-picked style silently reset.
    // Empty style is invalid DXF, so keep the default fallback.
    if (!t->style().empty()) {
        tex.style = t->style();
    }
    // Issue #412 phase 2: alignment point (code 11) was defaulting to (0,0,0),
    // which means the reader anchors non-left/non-baseline text at the origin
    // instead of at the intended insertion point.  libdxfrw's writeText only
    // emits code 11 when alignment is non-default, so seeding secPoint with
    // the insertion point costs nothing when unused and fixes placement when
    // it matters.
    tex.secPoint = tex.basePoint;

    dxfW->writeText(&tex);
}

std::vector<std::shared_ptr<DRW_Variant>> DXFimpl::extendedDataFor(
    const lc::entity::MText_CSPtr& t) const {
    if (_shelf == nullptr) {
        return {};
    }

    const auto found = _shelf->mtextExtendedData.find(t->id());
    if (found == _shelf->mtextExtendedData.end()) {
        return {};
    }

    // An edited MText is a different text, and a column layout describing the
    // text that used to be there is worse than no column layout: the reader
    // would lay the new words out to the old measurements. The captured text
    // is what decides -- the entity is immutable, so an edit is a new entity
    // with a new id in the ordinary case, but setProperties keeps the id.
    if (found->second.text != t->text_value()) {
        return {};
    }

    // Handle references cannot be replayed. A 1005 points at another record by
    // its code 5, and every handle in this file was either minted fresh or
    // moved by the reservation pass, so the handle the variant carries names
    // whatever now happens to hold it -- or nothing. Linked columns are the
    // case that reaches this (ACAD_MTEXT_COLUMNS lists its continuation MTEXTs
    // by handle); static columns carry 1000, 1070 and 1040 only and come
    // through. A dangling reference is worse than a lost one, so the whole
    // capture is dropped rather than half of it.
    for (const auto& variant : found->second.data) {
        if (variant == nullptr) {
            return {};
        }
        if (variant->code() == 1005) {
            return {};
        }
    }

    return found->second.data;
}

void DXFimpl::writeMText(const lc::entity::MText_CSPtr& t) {
    DRW_MText tex;
    getEntityAttributes(&tex, t);

    // Escape first, then break lines: plainToMText doubles a user's backslash
    // before it can be mistaken for the one in a \P this same call wrote. The
    // loop this replaces did neither -- it substituted \P for each newline and
    // left every other backslash alone, so a path like C:\Path went out as a
    // paragraph break in any conforming reader.
    const std::string correctedText = lc::persistence::plainToMText(t->text_value());

    // Issue #412 phase 2: preserve Z on insertion point.
    tex.basePoint.x = t->insertion_point().x();
    tex.basePoint.y = t->insertion_point().y();
    tex.basePoint.z = t->insertion_point().z();
    tex.text = correctedText;
    tex.height = t->height();
    tex.angle = t->angle() * 180 / M_PI;
    if (!t->style().empty()) {
        tex.style = t->style();
    }

    // Issue #412 phase 2: DXF group-code semantics differ between TEXT and
    // MTEXT.  For MTEXT, code 71 is the ATTACHMENT POINT (1..9, TL..BR), not
    // a text generation flag; code 72 is the DRAWING DIRECTION (1=LtR,
    // 3=TtB, 5=byStyle), not horizontal alignment; code 73 is the LINE
    // SPACING STYLE (1=at least, 2=exact), not vertical alignment.  The
    // previous writer passed halign/valign/textgeneration straight through
    // into 71/72/73 — reader-side addMText then reversed that with its
    // %3 and /3 rowcode/colcode arithmetic, so on round-trip an MText's
    // alignment was consistently garbled.  Compute the attachment point
    // from HAlign * VAlign the way addMText decodes it.
    int col;
    switch (t->halign()) {
        case lc::TextConst::HAlign::HALeft:   col = 1; break;
        case lc::TextConst::HAlign::HACenter: col = 2; break;
        case lc::TextConst::HAlign::HARight:  col = 3; break;
        default:                              col = 1; break;
    }
    int row;
    switch (t->valign()) {
        case lc::TextConst::VAlign::VATop:    row = 1; break;
        case lc::TextConst::VAlign::VAMiddle: row = 2; break;
        case lc::TextConst::VAlign::VABottom: row = 3; break;
        case lc::TextConst::VAlign::VABaseline: /* fall through */
        default:                              row = 3; break;
    }
    tex.textgen = (row - 1) * 3 + col;   // MText attachment point, code 71

    // Drawing direction, code 72: 1 left-to-right, 3 top-to-bottom, 5 by
    // style.  The entity now carries the real thing, so this is the value it
    // was read with -- previously it was derived from textgeneration(), which
    // is TEXT's group-71 mirroring flag and has no MTEXT meaning, and every
    // MText the dialog creates fell through to a hardcoded 5.
    //
    // libdxfrw types alignH/alignV as enums (DRW_Text::HAlign/VAlign) even on
    // the DRW_MText path where the DXF semantics have nothing to do with TEXT
    // alignment.  The cast is required: the integer set here is what the
    // writer emits verbatim as code 72.
    tex.alignH = static_cast<DRW_Text::HAlign>(t->drawingDirection());

    // Reference rectangle width, code 41.
    //
    // DRW_Text's constructor sets widthscale to 1 and dxfRW::writeMText emits
    // the group unconditionally, so leaving it alone declared a wrap column
    // one drawing unit wide on every MTEXT LibreCAD_3 has ever written -- and
    // the text dialog defaults the height to 100.  The entity now carries the
    // width it was read with, and 0 -- the default for one this build creates
    // -- is the documented "no reference rectangle, do not wrap".
    //
    // The field is called widthscale because it is TEXT's width factor; on an
    // MTEXT libdxfrw routes code 41 into the same member with an entirely
    // different meaning.
    tex.widthscale = t->width();

    // Line spacing style is code 73, which libdxfrw emits from
    // linespacingStyle -- not from alignV, which this used to set and which
    // the MTEXT write path never reads.  DXF numbers the two styles 1 and 2
    // and the enum numbers them 0 and 1, so this is an offset, not a cast.
    tex.linespacingStyle =
        (t->lineSpacingStyle() == lc::TextConst::LineSpacingStyle::Exact) ? 2 : 1;

    // Line spacing factor, code 44.
    tex.interlin = t->lineSpacingFactor();

    tex.extData = extendedDataFor(t);

    dxfW->writeMText(&tex);
}

void DXFimpl::writeMTextAsTextLines(const lc::entity::MText_CSPtr& t) {
    // One TEXT per line.  A TEXT record holds one line and has no line
    // spacing, so the spacing has to become geometry: each line gets its own
    // insertion point, stepped along the text's own "up" so a rotated MText
    // still reads as a rotated block.
    //
    // Every line carries the block's alignment, which is what makes the
    // placement work without repeating the renderer's layout: a reader anchors
    // each TEXT by that alignment, so stepping the anchors by one pitch puts
    // the lines where the block had them.  Where the FIRST line's anchor goes
    // is the only thing the alignment changes.
    std::vector<std::string> lines;
    {
        const std::string& text = t->text_value();
        std::string current;
        for (const char c : text) {
            if (c == '\n') {
                lines.push_back(current);
                current.clear();
            } else if (c != '\r') {
                current += c;
            }
        }
        lines.push_back(current);
    }

    const auto count = static_cast<double>(lines.size());
    // Including group 44: here the spacing IS the geometry, so a text that asks
    // for double spacing must come out of R12 double spaced. Ignoring the
    // factor collapsed every one of them to single spacing.
    const double pitch =
        lc::TextConst::mtextLinePitch(t->height(), t->lineSpacingFactor());

    // In pitches above the block's anchor.
    double firstLine = 0.0;
    switch (t->valign()) {
    case lc::TextConst::VAlign::VATop:
        firstLine = 0.0;               // the block hangs below its anchor
        break;
    case lc::TextConst::VAlign::VAMiddle:
        firstLine = (count - 1.0) / 2.0;
        break;
    case lc::TextConst::VAlign::VABottom:
    case lc::TextConst::VAlign::VABaseline:
    default:
        firstLine = count - 1.0;       // the block stands on its anchor
        break;
    }

    // The text's own up direction, so the lines stack along the MText's
    // rotation rather than along world Y.
    const double upX = -std::sin(t->angle());
    const double upY = std::cos(t->angle());

    for (std::size_t i = 0; i < lines.size(); i++) {
        const double pitches = firstLine - static_cast<double>(i);

        DRW_Text tex;
        getEntityAttributes(&tex, t);

        tex.basePoint.x = t->insertion_point().x() + upX * pitches * pitch;
        tex.basePoint.y = t->insertion_point().y() + upY * pitches * pitch;
        tex.basePoint.z = t->insertion_point().z();
        tex.text = lines[i];
        tex.height = t->height();
        tex.angle = t->angle() * 180 / M_PI;
        tex.alignH = DRW_Text::HAlign(t->halign());
        tex.alignV = DRW_Text::VAlign(t->valign());
        if (!t->style().empty()) {
            tex.style = t->style();
        }
        // See writeText: libdxfrw only emits code 11 for a non-default
        // alignment, and without it such text anchors at the origin.
        tex.secPoint = tex.basePoint;

        dxfW->writeText(&tex);
    }

    if (_mtextsAsText++ == 0) {
        _loss.notes.push_back(
            "Multiline text was written as one TEXT record per line: this "
            "revision has no MTEXT. The words and their placement are kept; "
            "the block, its reference width and its line spacing are not.");
    }
}

void DXFimpl::writeEntities() {
    for(const auto& e :_document->entityContainer().asVector()) {
        if(e->block() != nullptr) {
            continue;
        }

        writeEntity(e);
    }

    // The ENTITIES-section records LibreCAD has no model for, put back verbatim
    // after the entities it does model -- the same thing writeObjects() does for
    // the OBJECTS section. Without this they were captured on read, counted in
    // PreservedRecords::total() and had their handles reserved so the typed
    // writers would not reuse them, and were then silently absent from the
    // file: the write behaved as though they were being re-emitted while
    // dropping them. GEOPOSITIONMARKER, SECTIONOBJECT, ACAD_PROXY_ENTITY and
    // every other unmodelled 0-record in ENTITIES arrives here.
    if (_replay != nullptr) {
        for (const auto& entity : _replay->entities) {
            DRW_RawDxfObject copy = entity;
            if (!dxfW->writeRawDxfObject(&copy)) {
                LOG_ERROR << "libdxfrw refused to re-emit a preserved "
                          << entity.name << " record";
                return;
            }
        }
    }
}

/**
 * The DXF record an entity is written as, or "" when LibreCAD has no writer.
 *
 * Only the kinds a revision can refuse are named: everything else is carried by
 * every revision LibreCAD writes, so asking about it would be noise.
 */
static std::string recordKindOf(const lc::entity::CADEntity_CSPtr& entity) {
    if (std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
        return "SPLINE";
    }
    if (std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
        return "MTEXT";
    }
    if (std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
        return "HATCH";
    }
    if (std::dynamic_pointer_cast<const lc::entity::Image>(entity)) {
        return "IMAGE";
    }
    // R12 has no DIMENSION: libdxfrw's writeDimension refuses the record for
    // AC1009 and fails the whole emit, so without naming it here a drawing
    // holding a single dimension could not be saved to R12 at all -- no file,
    // and nothing in the result to say why.
    if (std::dynamic_pointer_cast<const lc::entity::Dimension>(entity)) {
        return "DIMENSION";
    }

    return "";
}

void DXFimpl::writeEntity(const lc::entity::CADEntity_CSPtr& entity) {
    // A revision that cannot hold this record is not a reason to lose the
    // drawing. libdxfrw refuses such an entity and the refusal fails the whole
    // write, so offering it one is the difference between a file missing one
    // spline and no file at all. Skipped here, counted, and reported.
    const auto recordKind = recordKindOf(entity);
    if (!recordKind.empty()
        && !variantCarriesRecord(File::variantIdForType(_exportType), recordKind)) {
        // MTEXT is the one of these that can be said another way. R12 has no
        // MTEXT and has had TEXT since the beginning, so a multiline note
        // becomes one TEXT per line rather than nothing at all -- losing the
        // block is not a reason to lose the words.
        if (auto mtext = std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
            writeMTextAsTextLines(mtext);
            return;
        }
        _loss.droppedByType[recordKind]++;
        return;
    }

    auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
    if (line != nullptr) {
        writeLine(line);
        return;
    }

    auto circle = std::dynamic_pointer_cast<const lc::entity::Circle>(entity);
    if (circle != nullptr) {
        writeCircle(circle);
        return;
    }

    auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity);
    if (arc != nullptr) {
        writeArc(arc);
        return;
    }

    auto image = std::dynamic_pointer_cast<const lc::entity::Image>(entity);
    if (image != nullptr) {
        writeImage(image);
        return;
    }

    auto lwPolyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity);
    if (lwPolyline != nullptr) {
        writeLWPolyline(lwPolyline);
        return;
    }

    auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity);
    if (text != nullptr) {
        writeText(text);
        return;
    }

    auto mtext = std::dynamic_pointer_cast<const lc::entity::MText>(entity);
    if (mtext != nullptr) {
        writeMText(mtext);
        return;
    }

    auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);
    if (ellipse != nullptr) {
        writeEllipse(ellipse);
        return;
    }

    auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(entity);
    if (insert != nullptr) {
        writeInsert(insert);
        return;
    }

    // Issue #412 phase 1: the six kinds the writer used to drop.
    // Point + Spline had complete writer bodies but were never dispatched;
    // Hatch had no writer at all.  Dimensions must dispatch per subtype
    // (not to the Dimension base) because the DXF group 70 subtype flag
    // and the subtype-specific geometry live on the concrete class.
    auto point = std::dynamic_pointer_cast<const lc::entity::Point>(entity);
    if (point != nullptr) {
        writePoint(point);
        return;
    }

    auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(entity);
    if (spline != nullptr) {
        writeSpline(spline);
        return;
    }

    auto hatch = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity);
    if (hatch != nullptr) {
        writeHatch(hatch);
        return;
    }

    // Per-subtype dimension dispatch — see writeDimensionCommon for why the
    // base class Dimension is never dispatched directly.
    auto dimLinear = std::dynamic_pointer_cast<const lc::entity::DimLinear>(entity);
    if (dimLinear != nullptr) {
        writeDimLinear(dimLinear);
        return;
    }

    auto dimAligned = std::dynamic_pointer_cast<const lc::entity::DimAligned>(entity);
    if (dimAligned != nullptr) {
        writeDimAligned(dimAligned);
        return;
    }

    auto dimRadial = std::dynamic_pointer_cast<const lc::entity::DimRadial>(entity);
    if (dimRadial != nullptr) {
        writeDimRadial(dimRadial);
        return;
    }

    auto dimDiametric = std::dynamic_pointer_cast<const lc::entity::DimDiametric>(entity);
    if (dimDiametric != nullptr) {
        writeDimDiametric(dimDiametric);
        return;
    }

    auto dimAngular = std::dynamic_pointer_cast<const lc::entity::DimAngular>(entity);
    if (dimAngular != nullptr) {
        writeDimAngular(dimAngular);
        return;
    }

    auto dimOrdinate = std::dynamic_pointer_cast<const lc::entity::DimOrdinate>(entity);
    if (dimOrdinate != nullptr) {
        writeDimOrdinate(dimOrdinate);
        return;
    }
}

/// Whether a block name follows the *D<n> convention writers use for the
/// anonymous blocks that hold a dimension's geometry.
///
/// A name alone does not make a block regenerable -- see
/// isRegeneratedDimensionBlock() -- but it is what the minting loop must not
/// collide with, so the two callers need the test on its own as well.
bool DXFimpl::isAnonymousDimensionBlockName(const std::string& name) {
    if(name.size() < 3 || name[0] != '*' || (name[1] != 'D' && name[1] != 'd')) {
        return false;
    }
    return name.find_first_not_of("0123456789", 2) == std::string::npos;
}

/// Whether this writer will mint a replacement for the given block, and may
/// therefore leave it out.
///
/// Both halves matter. Without the name test an anonymous block of some other
/// kind -- a *U<n> group, a hatch's boundary -- would be dropped with nothing
/// to take its place. Without the anonymous flag, a block that merely looks
/// like a dimension block is deleted along with everything in it: the drawing
/// opens, the INSERT that placed it now points at freshly minted dimension
/// geometry, and it draws arrowheads where the user put a circle.
bool DXFimpl::isRegeneratedDimensionBlock(const lc::meta::Block_CSPtr& block) {
    return block->anonymous() && isAnonymousDimensionBlockName(block->name());
}

void DXFimpl::writeBlockRecords() {
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    const bool omitUnusedDefaultPaper = mayOmitUnusedDefaultPaper();
#endif
    for(const auto& block : _document->blocks()) {
        // Anonymous dimension blocks are geometry a writer draws a dimension
        // with, not anything a user made. This writer mints its own below, so
        // the ones carried in from the source file are left out: emitting them
        // collides with the minted names -- libdxfrw refuses a duplicate
        // BLOCK_RECORD and the whole save is refused -- and keeping them would
        // grow the drawing by a block per dimension on every save-and-reopen.
        if(isRegeneratedDimensionBlock(block)) {
            continue;
        }
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        if ((omitUnusedDefaultPaper && block->name() == "*Paper_Space0") ||
            (!_defaultLayouts.empty() && (block->name() == "*Paper_Space" || isModelSpaceName(block->name())))) continue;
#endif
        dxfW->writeBlockRecord(block->name());
    }

    // A block needs its record. The names are minted from the same ordered walk
    // writeBlocks() uses, so the two agree without either having run yet -- both
    // start from the same base, which is why the base is computed here and left
    // in the member for writeBlocks() to pick up rather than recomputed there.
    //
    // The base has to clear every *D name that survived the loop above.
    // libdxfrw refuses a duplicate BLOCK_RECORD name and fails the whole write,
    // so a minted name landing on a kept block loses the entire save, silently.
    // The blocks this writer regenerates are gone by here and cannot collide;
    // it is the ones it deliberately kept -- user content under a *D name, an
    // anonymous block from a file this build does not model -- that the count
    // has to start above.
    _nextDimensionBlock = 1;
    for(const auto& block : _document->blocks()) {
        if(isRegeneratedDimensionBlock(block) || !isAnonymousDimensionBlockName(block->name())) {
            continue;
        }
        try {
            const unsigned long used = std::stoul(block->name().substr(2));
            if(used >= _nextDimensionBlock) {
                _nextDimensionBlock = static_cast<unsigned int>(used) + 1;
            }
        } catch(const std::exception&) {
            // A suffix too long to be a number cannot be one this writer mints.
        }
    }
    unsigned int next = _nextDimensionBlock;
    for(std::size_t i = 0; i < allDimensions().size(); i++) {
        dxfW->writeBlockRecord("*D" + std::to_string(next++));
    }
}


std::vector<lc::entity::CADEntity_CSPtr> DXFimpl::allDimensions() const {
    std::vector<lc::entity::CADEntity_CSPtr> dimensions;

    // Both callers use this to mint the anonymous *D blocks a DIMENSION draws
    // itself with. A revision that cannot carry DIMENSION gets none: the
    // records are skipped and reported as loss, so minting blocks for them
    // would leave the file holding anonymous blocks nothing references.
    if (_exportVersion <= DRW::AC1009) {
        return dimensions;
    }

    const auto collect = [&dimensions](const std::vector<lc::entity::CADEntity_CSPtr>& entities) {
        for (const auto& entity : entities) {
            if (std::dynamic_pointer_cast<const lc::entity::Dimension>(entity) != nullptr) {
                dimensions.push_back(entity);
            }
        }
    };

    collect(_document->entityContainer().asVector());
    for (const auto& block : _document->blocks()) {
        collect(_document->entitiesByBlock(block).asVector());
    }

    return dimensions;
}

namespace {

// DXF's own defaults for the parts of a dimension's appearance LibreCAD does
// not model. $DIMASZ, $DIMEXE and $DIMEXO are style variables; LibreCAD has no
// dimension style to read them from -- addDimStyle is a loss counter -- so the
// spec defaults stand in, and the geometry is at least self-consistent.
constexpr double kArrowSize = 0.18;
constexpr double kExtensionBeyond = 0.18;   // $DIMEXE: past the dimension line
constexpr double kExtensionOffset = 0.0625; // $DIMEXO: away from the measured point

struct Vector2 {
    double x{0.0};
    double y{0.0};
};

Vector2 operator-(const Vector2& a, const Vector2& b) {
    return {a.x - b.x, a.y - b.y};
}

Vector2 operator+(const Vector2& a, const Vector2& b) {
    return {a.x + b.x, a.y + b.y};
}

Vector2 operator*(const Vector2& v, double scale) {
    return {v.x * scale, v.y * scale};
}

double dot(const Vector2& a, const Vector2& b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 normalised(const Vector2& v) {
    const double length = std::sqrt(v.x * v.x + v.y * v.y);
    return length > 0.0 ? Vector2{v.x / length, v.y / length} : Vector2{1.0, 0.0};
}

Vector2 flat(const lc::geo::Coordinate& c) {
    return {c.x(), c.y()};
}

}  // namespace

namespace {

/**
 * The text a dimension shows: its own override, or the measurement.
 *
 * "<>" in the override stands for the measurement, as it does in DXF group 1.
 * It is also what LibreCAD's dimension builders start with, so written out
 * as it is, a dimension drawn in LibreCAD said "<>" to whoever opened the file.
 */
std::string dimensionText(const lc::entity::Dimension& dimension, double measured,
                          const std::string& prefix) {
    char formatted[32];
    std::snprintf(formatted, sizeof(formatted), "%g", measured);
    const std::string measurement = prefix + formatted;

    std::string text = dimension.explicitValue();
    if (text.empty()) {
        return measurement;
    }

    const auto placeholder = text.find("<>");
    if (placeholder != std::string::npos) {
        text.replace(placeholder, 2, measurement);
    }
    return text;
}

}  // namespace

std::string DXFimpl::beginDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                         std::string& layerName) {
    const std::string name = "*D" + std::to_string(_nextDimensionBlock++);

    DRW_Block block;
    block.name = name;
    block.basePoint = DRW_Coord(0.0, 0.0, 0.0);
    // 1 = anonymous: machinery, not a block to offer the user in a list.
    block.flags = 1;
    if (!dxfW->writeBlock(&block)) {
        LOG_ERROR << "libdxfrw refused the dimension block " << name;
        return "";
    }

    layerName = entity->layer() != nullptr ? entity->layer()->name() : std::string("0");
    return name;
}

void DXFimpl::writeDimensionLine(const std::string& layerName,
                                 double fromX, double fromY, double toX, double toY) {
    DRW_Line drw;
    drw.layer = layerName;
    drw.basePoint = DRW_Coord(fromX, fromY, 0.0);
    drw.secPoint = DRW_Coord(toX, toY, 0.0);
    dxfW->writeLine(&drw);
}

void DXFimpl::writeDimensionArrow(const std::string& layerName,
                                  double tipX, double tipY, double dirX, double dirY) {
    const double backX = tipX + dirX * kArrowSize;
    const double backY = tipY + dirY * kArrowSize;
    const double sideX = -dirY * kArrowSize * 0.5;
    const double sideY = dirX * kArrowSize * 0.5;

    DRW_Solid drw;
    drw.layer = layerName;
    drw.basePoint = DRW_Coord(tipX, tipY, 0.0);
    drw.secPoint = DRW_Coord(backX + sideX, backY + sideY, 0.0);
    // A DXF SOLID's corners run in a bow-tie, which is the same ordering
    // addSolid reads back.
    drw.thirdPoint = DRW_Coord(backX - sideX, backY - sideY, 0.0);
    drw.fourPoint = drw.thirdPoint;
    dxfW->writeSolid(&drw);
}

void DXFimpl::writeDimensionText(const std::string& layerName,
                                 const lc::entity::Dimension& dimension,
                                 const std::string& value) {
    // A single space is DXF's way of saying "draw no text".
    if (value == " ") {
        return;
    }

    DRW_Text text;
    text.layer = layerName;
    text.basePoint = DRW_Coord(dimension.middleOfText().x(), dimension.middleOfText().y(), 0.0);
    text.secPoint = text.basePoint;
    text.height = kArrowSize * 2.0;
    text.text = value;
    text.angle = dimension.textAngle();
    text.alignH = DRW_Text::HCenter;
    text.alignV = DRW_Text::VMiddle;
    text.style = "STANDARD";
    dxfW->writeText(&text);
}

std::string DXFimpl::writeLeaderDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                               const lc::entity::Dimension& dimension,
                                               double fromX, double fromY,
                                               double toX, double toY,
                                               bool arrowAtBothEnds,
                                               const std::string& prefix) {
    const Vector2 from{fromX, fromY};
    const Vector2 to{toX, toY};

    std::string layerName;
    const std::string name = beginDimensionBlock(entity, layerName);
    if (name.empty()) {
        return "";
    }

    writeDimensionLine(layerName, from.x, from.y, to.x, to.y);

    const Vector2 direction = normalised(to - from);
    writeDimensionArrow(layerName, to.x, to.y, direction.x * -1.0, direction.y * -1.0);
    if (arrowAtBothEnds) {
        writeDimensionArrow(layerName, from.x, from.y, direction.x, direction.y);
    }

    const Vector2 span = to - from;
    writeDimensionText(layerName, dimension,
                       dimensionText(dimension, std::sqrt(dot(span, span)), prefix));

    return name;
}

std::string DXFimpl::writeAngularDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                                const lc::entity::Dimension& dimension,
                                                const lc::entity::DimAngular& angular) {
    // The two measured lines meet at a vertex; the dimension draws an arc
    // between them, through the point the dimension itself sits at.
    const Vector2 firstStart = flat(angular.defLine11());
    const Vector2 firstEnd = flat(angular.defLine12());
    const Vector2 secondStart = flat(angular.defLine21());
    const Vector2 secondEnd = flat(angular.defLine22());
    const Vector2 through = flat(dimension.definitionPoint());

    // The vertex is where the two lines cross. Solving it directly keeps this
    // honest for lines that do not share an endpoint.
    const Vector2 firstDir = normalised(firstEnd - firstStart);
    const Vector2 secondDir = normalised(secondEnd - secondStart);
    const double denominator = firstDir.x * secondDir.y - firstDir.y * secondDir.x;
    if (std::abs(denominator) < 1e-12) {
        // Parallel lines subtend no angle; there is no arc to draw.
        return "";
    }

    const Vector2 between = secondStart - firstStart;
    const double along = (between.x * secondDir.y - between.y * secondDir.x) / denominator;
    const Vector2 vertex = firstStart + firstDir * along;

    std::string layerName;
    const std::string name = beginDimensionBlock(entity, layerName);
    if (name.empty()) {
        return "";
    }

    const Vector2 toArc = through - vertex;
    const double radius = std::sqrt(dot(toArc, toArc));
    if (radius <= 0.0) {
        return name;
    }

    double startAngle = std::atan2(firstEnd.y - vertex.y, firstEnd.x - vertex.x);
    double endAngle = std::atan2(secondEnd.y - vertex.y, secondEnd.x - vertex.x);

    // Extension lines out to the arc, then the arc itself.
    writeDimensionLine(layerName, firstEnd.x, firstEnd.y,
                       vertex.x + std::cos(startAngle) * (radius + kExtensionBeyond),
                       vertex.y + std::sin(startAngle) * (radius + kExtensionBeyond));
    writeDimensionLine(layerName, secondEnd.x, secondEnd.y,
                       vertex.x + std::cos(endAngle) * (radius + kExtensionBeyond),
                       vertex.y + std::sin(endAngle) * (radius + kExtensionBeyond));

    DRW_Arc arc;
    arc.layer = layerName;
    arc.basePoint = DRW_Coord(vertex.x, vertex.y, 0.0);
    arc.radious = radius;
    arc.staangle = startAngle;
    arc.endangle = endAngle;
    arc.isccw = 1;
    dxfW->writeArc(&arc);

    double swept = endAngle - startAngle;
    while (swept < 0.0) {
        swept += 2.0 * M_PI;
    }

    char formatted[32];
    std::snprintf(formatted, sizeof(formatted), "%g", swept * 180.0 / M_PI);
    writeDimensionText(layerName, dimension,
                       dimension.explicitValue().empty()
                           ? std::string(formatted) + "\xc2\xb0" : dimension.explicitValue());

    return name;
}

std::string DXFimpl::writeOrdinateDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                                 const lc::entity::DimOrdinate& ordinate) {
    // The leader leaves the feature at right angles to the measured axis --
    // vertically for an X ordinate -- and, when its end is off to the side,
    // jogs across to it over the middle third: the picture LibreCAD draws.
    const Vector2 feature = flat(ordinate.featurePoint());
    const Vector2 leaderEnd = flat(ordinate.leaderEndPoint());
    const Vector2 alongAxis = ordinate.xType() ? Vector2{0.0, 1.0} : Vector2{1.0, 0.0};
    const Vector2 asideAxis = ordinate.xType() ? Vector2{1.0, 0.0} : Vector2{0.0, 1.0};
    const double along = dot(leaderEnd - feature, alongAxis);
    const double aside = dot(leaderEnd - feature, asideAxis);

    std::string layerName;
    const std::string name = beginDimensionBlock(entity, layerName);
    if (name.empty()) {
        return "";
    }

    // Like an extension line, the leader starts just off the feature.
    Vector2 start = feature;
    if (std::abs(along) / 3.0 > kExtensionOffset) {
        start = feature + alongAxis * (along < 0.0 ? -kExtensionOffset : kExtensionOffset);
    }

    if (std::abs(aside) > 1e-9) {
        const Vector2 firstBend = feature + alongAxis * (along / 3.0);
        const Vector2 secondBend = feature + alongAxis * (along * 2.0 / 3.0) + asideAxis * aside;
        writeDimensionLine(layerName, start.x, start.y, firstBend.x, firstBend.y);
        writeDimensionLine(layerName, firstBend.x, firstBend.y, secondBend.x, secondBend.y);
        writeDimensionLine(layerName, secondBend.x, secondBend.y, leaderEnd.x, leaderEnd.y);
    } else {
        writeDimensionLine(layerName, start.x, start.y, leaderEnd.x, leaderEnd.y);
    }

    // Without its sign, as AutoCAD writes an ordinate.
    const std::string value = dimensionText(ordinate, std::abs(ordinate.value()), "");
    if (value == " ") {  // a single space is DXF's "suppress the text"
        return name;
    }

    // Placed the way groups 71 and 11 say, which for an ordinate LibreCAD
    // drew is past the end of the leader: the attachment point names the point
    // of the text that sits at the text position, 1 to 9 across and then down.
    static const DRW_Text::HAlign across[] = {DRW_Text::HLeft, DRW_Text::HCenter, DRW_Text::HRight};
    static const DRW_Text::VAlign down[] = {DRW_Text::VTop, DRW_Text::VMiddle, DRW_Text::VBottom};
    const int attachment = static_cast<int>(ordinate.attachmentPoint());
    const bool known = attachment >= 1 && attachment <= 9;

    DRW_Text text;
    text.layer = layerName;
    text.basePoint = DRW_Coord(ordinate.middleOfText().x(), ordinate.middleOfText().y(), 0.0);
    text.secPoint = text.basePoint;
    text.height = kArrowSize * 2.0;
    text.text = value;
    text.angle = ordinate.textAngle() * 180.0 / M_PI;
    text.alignH = known ? across[(attachment - 1) % 3] : DRW_Text::HCenter;
    text.alignV = known ? down[(attachment - 1) / 3] : DRW_Text::VMiddle;
    text.style = "STANDARD";
    dxfW->writeText(&text);

    return name;
}

std::string DXFimpl::writeDimensionBlock(const lc::entity::CADEntity_CSPtr& entity) {
    const auto dimension = std::dynamic_pointer_cast<const lc::entity::Dimension>(entity);
    if (dimension == nullptr) {
        return "";
    }

    // The two measured points and the direction the dimension line runs in.
    // For an aligned dimension that direction is the line between the points;
    // for a linear one it is the dimension's own rotation, which is what makes
    // a horizontal dimension over a sloped feature come out horizontal.
    Vector2 first;
    Vector2 second;
    Vector2 along;

    if (auto linear = std::dynamic_pointer_cast<const lc::entity::DimLinear>(entity)) {
        first = flat(linear->definitionPoint2());
        second = flat(linear->definitionPoint3());
        along = {std::cos(linear->angle()), std::sin(linear->angle())};
    } else if (auto aligned = std::dynamic_pointer_cast<const lc::entity::DimAligned>(entity)) {
        first = flat(aligned->definitionPoint2());
        second = flat(aligned->definitionPoint3());
        along = normalised(second - first);
    } else if (auto radial = std::dynamic_pointer_cast<const lc::entity::DimRadial>(entity)) {
        // A leader from the centre out to the edge, with one arrowhead where it
        // meets the circle.
        return writeLeaderDimensionBlock(entity, *dimension,
                                         radial->definitionPoint().x(), radial->definitionPoint().y(),
                                         radial->definitionPoint2().x(), radial->definitionPoint2().y(),
                                         false, "R");
    } else if (auto diametric = std::dynamic_pointer_cast<const lc::entity::DimDiametric>(entity)) {
        // A line right across the circle, arrowheads at both ends.
        return writeLeaderDimensionBlock(entity, *dimension,
                                         diametric->definitionPoint().x(), diametric->definitionPoint().y(),
                                         diametric->definitionPoint2().x(), diametric->definitionPoint2().y(),
                                         true, "");
    } else if (auto angular = std::dynamic_pointer_cast<const lc::entity::DimAngular>(entity)) {
        return writeAngularDimensionBlock(entity, *dimension, *angular);
    } else if (auto ordinate = std::dynamic_pointer_cast<const lc::entity::DimOrdinate>(entity)) {
        return writeOrdinateDimensionBlock(entity, *ordinate);
    } else {
        return "";
    }

    const Vector2 onLine = flat(dimension->definitionPoint());
    const Vector2 across = {-along.y, along.x};

    // Where each measured point meets the dimension line: slide it along the
    // perpendicular until it is level with the line.
    const double offsetFirst = dot(onLine - first, across);
    const double offsetSecond = dot(onLine - second, across);
    const Vector2 footFirst = first + across * offsetFirst;
    const Vector2 footSecond = second + across * offsetSecond;

    const std::string name = "*D" + std::to_string(_nextDimensionBlock++);

    DRW_Block block;
    block.name = name;
    block.basePoint = DRW_Coord(0.0, 0.0, 0.0);
    // 1 = anonymous: this block is machinery, not something to offer the user
    // in a block list.
    block.flags = 1;
    if (!dxfW->writeBlock(&block)) {
        LOG_ERROR << "libdxfrw refused the dimension block " << name;
        return "";
    }

    const auto layerName = entity->layer() != nullptr ? entity->layer()->name() : std::string("0");

    const auto line = [&](const Vector2& from, const Vector2& to) {
        DRW_Line drw;
        drw.layer = layerName;
        drw.basePoint = DRW_Coord(from.x, from.y, 0.0);
        drw.secPoint = DRW_Coord(to.x, to.y, 0.0);
        dxfW->writeLine(&drw);
    };

    // An arrowhead as a filled triangle, pointing at `tip` from `direction`.
    const auto arrow = [&](const Vector2& tip, const Vector2& direction) {
        const Vector2 back = tip + direction * kArrowSize;
        const Vector2 side = {-direction.y * kArrowSize * 0.5, direction.x * kArrowSize * 0.5};

        DRW_Solid drw;
        drw.layer = layerName;
        drw.basePoint = DRW_Coord(tip.x, tip.y, 0.0);
        drw.secPoint = DRW_Coord(back.x + side.x, back.y + side.y, 0.0);
        // A DXF SOLID's corners run in a bow-tie, so the third and fourth
        // points are the far edge in reverse -- the same ordering addSolid
        // reads back.
        drw.thirdPoint = DRW_Coord(back.x - side.x, back.y - side.y, 0.0);
        drw.fourPoint = drw.thirdPoint;
        dxfW->writeSolid(&drw);
    };

    // Extension lines: they start just off the measured point and run a little
    // past the dimension line, which is what makes them read as extensions
    // rather than as part of the drawing.
    const auto extension = [&](const Vector2& from, const Vector2& foot) {
        const Vector2 direction = normalised(foot - from);
        line(from + direction * kExtensionOffset, foot + direction * kExtensionBeyond);
    };

    extension(first, footFirst);
    extension(second, footSecond);
    line(footFirst, footSecond);

    const Vector2 inward = normalised(footSecond - footFirst);
    arrow(footFirst, inward);
    arrow(footSecond, inward * -1.0);

    // The measurement. An explicit value overrides it -- that is what DXF's
    // group 1 means -- and otherwise it is the distance along the dimension
    // line, which is the number the dimension exists to state.
    std::string measurement = dimension->explicitValue();
    if (measurement.empty()) {
        const double measured = std::abs(dot(footSecond - footFirst, inward));
        char formatted[32];
        std::snprintf(formatted, sizeof(formatted), "%g", measured);
        measurement = formatted;
    }

    if (measurement != " ") {  // a single space is DXF's "suppress the text"
        DRW_Text text;
        text.layer = layerName;
        text.basePoint = DRW_Coord(dimension->middleOfText().x(), dimension->middleOfText().y(), 0.0);
        text.secPoint = text.basePoint;
        text.height = kArrowSize * 2.0;
        text.text = measurement;
        text.angle = dimension->textAngle();
        text.alignH = DRW_Text::HCenter;
        text.alignV = DRW_Text::VMiddle;
        text.style = "STANDARD";
        dxfW->writeText(&text);
    }

    return name;
}

void DXFimpl::writeBlocks() {
    // One anonymous block per dimension first, holding the geometry it draws.
    // writeDimension() names that block from _dimensionBlocks, so the map has
    // to be complete before anything writes a DIMENSION -- and a dimension
    // inside a user block is written by the loop below, not by writeEntities().
    // Minting after the user blocks left those dimensions with no group-2 name
    // and the block they owned unreferenced: geometry in the file that nothing
    // drew, and a dimension the reader had to guess at.
    for(const auto& entity : allDimensions()) {
        const auto name = writeDimensionBlock(entity);
        if (!name.empty()) {
            _dimensionBlocks[entity->id()] = name;
        }
    }

#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    const bool omitUnusedDefaultPaper = mayOmitUnusedDefaultPaper();
#endif
    for(const auto& block : _document->blocks()) {
        // Left out for the same reason as in writeBlockRecords(): the record
        // pass skipped these, so a block written here would have no record.
        if(isRegeneratedDimensionBlock(block)) {
            continue;
        }
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
        if ((omitUnusedDefaultPaper && block->name() == "*Paper_Space0") ||
            (!_defaultLayouts.empty() && (block->name() == "*Paper_Space" || isModelSpaceName(block->name())))) continue;
#endif
        writeBlock(block);
    }
}

void DXFimpl::writeBlock(const lc::meta::Block_CSPtr& block) {
    DRW_Block drwBlock;

    drwBlock.name = block->name();
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    const bool sourceLayoutBlock = _defaultLayoutsFromSource && block->name() == "*Paper_Space0";
    if (sourceLayoutBlock) drwBlock.handle = _replay->layoutExtraBlockBegin.handle;
#endif
    // Code 70 bit 1. A kept anonymous block that goes out unmarked is
    // indistinguishable from user content on the next read, so it would be
    // kept forever -- and one that really is user content must not acquire the
    // mark and be dropped on the next save.
    drwBlock.flags = block->anonymous() ? 1 : 0;
    drwBlock.basePoint.x = block->base().x();
    drwBlock.basePoint.y = block->base().y();
    drwBlock.basePoint.z = block->base().z();

    auto customEntity = std::dynamic_pointer_cast<const lc::meta::CustomEntityStorage>(block);
    if(customEntity) {
        auto list = std::list<DRW_Variant>({
            DRW_Variant(APP_NAME_CODE, APP_NAME),
            DRW_Variant(PLUGIN_NAME_CODE, customEntity->pluginName()),
            DRW_Variant(ENTITY_NAME_CODE, customEntity->entityName()),
        });

        for(const auto& data : customEntity->params()) {
            list.emplace_back(DRW_Variant(470, data.first));
            list.emplace_back(DRW_Variant(471, data.second));
        }

        // Close the application group. dxfRW::writeAppData requires balanced
        // 102 markers and fails the whole write without this, so a document
        // holding a custom entity produced no file at all.
        list.emplace_back(DRW_Variant(APP_NAME_CODE, std::string("}")));

        drwBlock.appData.push_back(list);
    }

    dxfW->writeBlock(&drwBlock);

    for(const auto& entity : _document->entitiesByBlock(block).asVector()) {
        writeEntity(entity);
    }
#ifdef LC3_HAS_CHECKED_DEFAULT_LAYOUT_GRAPH
    if (sourceLayoutBlock && !dxfW->writeBlockEnd(&_replay->layoutExtraBlockEnd)) _defaultLayoutWriteOk = false;
#endif
}


/*****************************************
 * EXTRA Utilities
 *****************************************/
lc::AngleFormat DXFimpl::numberToAngleFormat(int num) {
    lc::AngleFormat af;

    switch (num) {
    default:
    case 0:
        af = lc::AngleFormat::DegreesDecimal;
        break;
    case 1:
        af = lc::AngleFormat::DegreesMinutesSeconds;
        break;
    case 2:
        af = lc::AngleFormat::Gradians;
        break;
    case 3:
        af = lc::AngleFormat::Radians;
        break;
    case 4:
        af = lc::AngleFormat::Surveyors;
        break;
    }

    return af;
}

/**
 * Converts AngleFormat enum to DXF number.
 */
int DXFimpl::angleFormatToNumber(lc::AngleFormat af) {
    int num;

    switch (af) {
    default:
    case lc::AngleFormat::DegreesDecimal:
        num = 0;
        break;
    case lc::AngleFormat::DegreesMinutesSeconds:
        num = 1;
        break;
    case lc::AngleFormat::Gradians:
        num = 2;
        break;
    case lc::AngleFormat::Radians:
        num = 3;
        break;
    case lc::AngleFormat::Surveyors:
        num = 4;
        break;
    }

    return num;
}

/**
 * converts a DXF units setting (e.g. INSUNITSs) to a units enum.
 */
lc::Units DXFimpl::numberToUnit(int num) {
    try {
        return _dxfToLCUnits.at(num);
    }
    catch (std::out_of_range& e) {
        return lc::Units::None;
    }
}

/**
 * Converts a units enum into a DXF units number e.g. for INSUNITSs.
 */
int DXFimpl::unitToNumber(lc::Units unit) {
    try {
        return _lcUnitsToDXF.at(unit);
    }
    catch (std::out_of_range& e) {
        return 0;
    }
}
