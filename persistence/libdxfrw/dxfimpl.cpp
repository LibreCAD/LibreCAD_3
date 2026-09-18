#include "dxfimpl.h"
#include "../generic/helpers.h"

#include "../patternLoader/patternProvider.h"
#include <algorithm>
#include <set>

#include <cad/primitive/circle.h>
#include <cad/primitive/hatch.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
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
#include <cad/logger/logger.h>
#include <cad/tools/maphelper.h>

using namespace lc::persistence;

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

        if (headerInt(*data, "$INSUNITS", _header.insUnitsCode)) {
            _header.units = numberToUnit(_header.insUnitsCode);
        }

        headerInt(*data, "$MEASUREMENT", _header.measurement);
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

        _currentBlock = nullptr;

        auto base = coord(data.basePoint);

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
                data.name, pluginName, entityName, base, params);
        }

        if(_currentBlock == nullptr) {
            _currentBlock = std::make_shared<lc::meta::Block>(data.name, base);
        }
        _builder->append(std::make_shared<lc::operation::AddBlock>(_document, _currentBlock));

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
        auto col = icol.intToColor(data.color);

        if (col == nullptr) {
            col = icol.intToColor(255);
        }

        auto lw = getLcLineWidth<lc::meta::MetaLineWidthByValue>(data.lWeight);

        if (lw == nullptr) {
            lw = getLcLineWidth<lc::meta::MetaLineWidthByValue>(DRW_LW_Conv::lineWidth::width00);
        }

        auto lp = _document->linePatternByName(data.lineType);
        auto isFrozen = (bool) ((unsigned int) data.flags & 1u);

        auto layer = std::make_shared<lc::meta::Layer>(data.name, lw->width(), col->color(), lp, isFrozen);
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
                        getBlock(*data)
                                                            );

        deliver(lcSpline);
    });
}

void DXFimpl::addText(const DRW_Text& data) {
    guarded("TEXT", &data, [&] {
        LOG_TRACE << "addText";
        auto layer = getLayer(data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);
        auto lcText = std::make_shared<lc::entity::Text>(coord(data.basePoint),
                      data.text, data.height,
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
                               data->getAngle(),
                               data->getOblique(),
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
        LOG_WARNING << "Dropping DIMENSION (ordinate): no kernel entity for it";
        recordLoss("DIMENSION (ordinate)");
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

void DXFimpl::addMText(const DRW_MText& data) {
    guarded("MTEXT", &data, [&] {
        LOG_TRACE << "addMText";
        auto layer = getLayer(data);
        std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(data);
        lc::TextConst::HAlign halign;
        lc::TextConst::VAlign valign;
        //lc::TextConst::AttachmentPoint attachmentPoint = lc::TextConst::AttachmentPoint(data.textgen);
        lc::TextConst::DrawingDirection drawingDir;
        //lc::TextConst::LineSpacingStyle lineSpacingStyle;

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

        // Uncomment when line spacing style has been implemented
        /*if (data.alignV == 1) {
            lineSpacingStyle = lc::TextConst::LineSpacingStyle::AtLeast;
        }
        else {
            lineSpacingStyle = lc::TextConst::LineSpacingStyle::Exact;
        }*/

        auto lcMText = std::make_shared<lc::entity::MText>(coord(data.basePoint),
                      data.text, data.height,
                      data.angle * M_PI / 180, data.style,
                      lc::TextConst::DrawingDirection(drawingDir),
                      lc::TextConst::HAlign(halign),
                      lc::TextConst::VAlign(valign),
                      false,
                      false,
                      false,
                      false,
                      layer,
                      mf,
                      getBlock(data)
                                                        );

        deliver(lcMText);
    });
}

void DXFimpl::addHatch(const DRW_Hatch* data) {
    guarded("HATCH", data, [&] {
        // Loop->objlist contains the 3 entities (copied) that define the hatch areas are the entities selected during hatch
        // loopList seems to contain the same entities, why??
        LOG_TRACE << "addHatch ";
        auto layer = getLayer(*data);
        auto mf = getMetaInfo(*data);
        lc::geo::Region reg;
        auto lcHatch = std::make_shared<lc::entity::Hatch>(   layer,
                       mf,
                       getBlock(*data)
                                                          );
        lcHatch->setPatternName(data->name);
        lcHatch->setSolid(data->solid);
        LOG_TRACE << "name " << data->name;
        LOG_TRACE << "solid " << data->solid;
        if(!data->solid) {
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
                        points.emplace_back(lc::geo::Coordinate(i->x, i->y), i->bulge, i->stawidth, i->endwidth);
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
                    builder.setStart(coord(data->basePoint));
                    builder.setEnd(coord(data->secPoint));
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
                    builder.setCenter(coord(data->basePoint));
                    builder.setRadius(data->radious);
                    builder.setStartAngle(data->staangle);
                    builder.setEndAngle(data->endangle);

                    builder.setIsCCW((bool) data->isccw);
                    builder.setLayer(layer);
                    loopData.push_back(builder.build());
                } else if(k->eType == DRW::ETYPE::ELLIPSE) { //done
                    auto data = std::dynamic_pointer_cast<DRW_Ellipse>(k);
                    auto secPoint = coord(data->secPoint);
                    auto lcEllipse = std::make_shared<lc::entity::Ellipse>(coord(data->basePoint),
                                     secPoint,
                                     secPoint.magnitude() * data->ratio,
                                     data->staparam,
                                     data->endparam,
                                     data->isccw,
                                     layer
                                                                          );
                    loopData.push_back(lcEllipse);
                } else if(k->eType == DRW::ETYPE::SPLINE) {
                    auto data = std::dynamic_pointer_cast<DRW_Spline>(k);
                    // Same as addSpline: keep the file's knot vector intact.
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
                                    getBlock(*data)
                                                                        );
                    loopData.push_back(lcSpline);
                }
            }
            lc::geo::Loop loop(loopData);
            reg.addLoop(loop);
        }
        lcHatch->setRegion(reg);
        deliver(lcHatch);
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

lc::meta::Layer_CSPtr DXFimpl::getLayer(const DRW_Entity& data) const {
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
    if(data.color == BYBLOCK_COLOR) {
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

void DXFimpl::linkImage(const DRW_ImageDef *data) {
    guarded("IMAGEDEF", nullptr, [&] {
        LOG_TRACE << "linkImage";
        for(auto image = imageMapCache.cbegin(); image != imageMapCache.cend() /* not hoisted */; /* no increment */ ) {
            if (image->ref == data->handle) {
                auto layer = _document->layerByName(image->layer);

                std::shared_ptr<lc::meta::MetaInfo> mf = getMetaInfo(*image);
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
        // position, which is what an ATTRIB draws, so each visible one arrives
        // as a Text. The tag does not survive, and a later save writes TEXT
        // rather than ATTRIB -- the note records that rather than leaving the
        // user to discover it on the next round trip.
        for (const auto& attrib : data.attlist) {
            if (attrib == nullptr) {
                continue;
            }

            // Bit 0 = invisible: AutoCAD does not draw these, so neither do we.
            if ((attrib->attribFlags & 1u) != 0u) {
                recordLoss("ATTRIB (invisible)");
                continue;
            }

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

            if (_attributesAsText++ == 0) {
                _loss.notes.push_back(
                    "Block attributes were imported as plain text: LibreCAD has no "
                    "attribute model, so their tags are not kept and a save writes "
                    "them as TEXT.");
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

void DXFimpl::writeObjects() {
    if (_replay == nullptr) {
        return;
    }

    // An IMAGE in the document makes the typed writer emit its own IMAGEDEF and
    // IMAGEDEF_REACTOR. Replaying the source's as well leaves the file with two
    // of each for one image, so the preserved copies give way to the regenerated
    // ones -- measured: without this, sample_AC1015_ascii comes back with two
    // reactors.
    bool documentHasImages = false;
    for (const auto& entity : _document->entityContainer().asVector()) {
        if (std::dynamic_pointer_cast<const lc::entity::Image>(entity)) {
            documentHasImages = true;
            break;
        }
    }

    for (const auto& object : _replay->objects) {
        if (documentHasImages
            && (object.name == "IMAGEDEF" || object.name == "IMAGEDEF_REACTOR")) {
            continue;
        }

        DRW_RawDxfObject copy = object;
        if (!dxfW->writeRawDxfObject(&copy)) {
            LOG_ERROR << "libdxfrw refused to re-emit a preserved " << object.name << " record";
            return;
        }
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
    lay.color = icol_inst.colorToInt(col);

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

    // Issue #412 phase 3: emit the layer's line pattern name (code 6) so the
    // reader can match entities that reference it back to the layer LTYPE.
    // Skip an empty/unset pattern; libdxfrw defaults to "CONTINUOUS".
    auto lp = layer->linePattern();
    if (lp != nullptr && !lp->name().empty()) {
        lay.lineType = lp->name();
    }

    lay.flags = layer->isFrozen() ? 0x01 : 0x00;

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
        return DRW::UNKNOWNV;
    }
    return DRW::UNKNOWNV;
}

void DXFimpl::attachPreservedRecords() {
    // The header travels even when there is nothing else to carry: a drawing's
    // units are worth keeping on their own.
    const bool hasHeader = !_header.acadVersion.empty() || _header.insUnitsCode != 0
                           || _header.measurement != 0;
    if (_preserved.empty() && _preserved.classes.empty() && !hasHeader) {
        return;
    }

    auto preserved = std::make_shared<PreservedRecords>(_preserved);
    preserved->hasHeader = hasHeader;
    preserved->insUnitsCode = _header.insUnitsCode;
    preserved->measurement = _header.measurement;
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

bool DXFimpl::writeDXF(const std::string& filename, lc::persistence::File::Type type) {
    dxfW = new dxfRW(filename.c_str());

    //Default setting
    DRW::Version exportVersion;

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
        // File::save refuses this before we are called; there is no DXF
        // revision to map it to.
        LOG_ERROR << "No DXF revision for file type " << static_cast<int>(type);
        return false;
    }

    // Was `< LIBDXFRW_DXB_R2013`, so the last binary target wrote ASCII.
    const bool isBinary = lc::persistence::File::isBinaryType(type);

    _exportVersion = exportVersion;
    _exportType = type;

    // Put back whatever the read set aside, if this is the same revision it
    // came from. A record valid in R2013 has no defined meaning in R12, and
    // re-emitting it there would produce a file that claims to be R12 and is
    // not -- so a down-convert drops them, and says how many.
    _replay = preservedOn(_document);
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
        //  * ASCII target: these are replayed from the source spellings the
        //    reader captured, and a binary write of them is refused by the
        //    library -- which fails the whole file, so it must not be tried.
        //  * Captured with those spellings: a record read from a binary DXF
        //    has empty placeholders instead, and cannot be re-emitted verbatim.
        const bool sameRevision = _replay->version == exportVersion;
        const bool verbatimAvailable =
            std::all_of(_replay->objects.begin(), _replay->objects.end(),
                        [](const DRW_RawDxfObject& object) { return object.hasRawValues; });

        if (!sameRevision || isBinary || !verbatimAvailable) {
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
        // Every preserved handle is claimed before the write starts, so the
        // handles the typed writers mint cannot collide with one being
        // re-emitted verbatim.
        for (const auto& object : _replay->objects) {
            if (object.handle != 0) {
                dxfW->reserveHandle(object.handle);
            }
        }
        for (const auto& entity : _replay->entities) {
            if (entity.handle != 0) {
                dxfW->reserveHandle(entity.handle);
            }
        }

        dxfW->setDxfClasses(_replay->classes);
        dxfW->setRawDxfSections(_replay->sections);
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

    if(metaColorByBlock != nullptr) {
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

    if(metaWidthByValue != nullptr) {
        ent->lWeight = static_cast<DRW_LW_Conv::lineWidth>(widthToInt(metaWidthByValue->width()));
    }
    else if(metaWidthByBlock != nullptr) {
        ent->lWeight = DRW_LW_Conv::lineWidth::widthByBlock;
    }
}

void DXFimpl::writeLTypes() {
    DRW_LType ltype;
    // Standard linetypes for LibreCAD / AutoCAD
    ltype.name = "CONTINUOUS";
    ltype.desc = "Solid line";
    dxfW->writeLineType(&ltype);
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
        data.addInt("$MEASUREMENT", preserved->measurement, 70);
        data.addInt("$INSUNITS", preserved->insUnitsCode, 70);
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
    data.addInt("$MEASUREMENT", 1, 70);       // 0 = English, 1 = Metric
    data.addInt("$INSUNITS", 4, 70);          // 4 = Millimeter
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
    dim.setAngle(d->angle());
    dim.setOblique(d->oblique());
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

    dxfW->writeImage(&img, i->name());
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
        edge->knotslist = spline->knotPoints();
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
            continue;
        }

        drwLoop->update();
        hatch.appendLoop(drwLoop);
    }

    if (hatch.looplist.empty()) {
        LOG_WARNING << "Dropping HATCH " << hatch.name << ": no boundary survived";
        return;
    }
    hatch.loopsnum = hatch.looplist.size();

    if (!dxfW->writeHatch(&hatch)) {
        LOG_ERROR << "libdxfrw refused HATCH " << hatch.name;
    }
}

void DXFimpl::writeText(const lc::entity::Text_CSPtr& t) {
    DRW_Text tex;
    getEntityAttributes(&tex, t);

    std::string correctedText = t->text_value();
    std::replace(correctedText.begin(), correctedText.end(), '\n', '\\');

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

void DXFimpl::writeMText(const lc::entity::MText_CSPtr& t) {
    DRW_MText tex;
    getEntityAttributes(&tex, t);

    std::string correctedText = t->text_value();
    size_t index = 0;
    while (correctedText.find('\n', index) != std::string::npos) {
        index = correctedText.find('\n');
        correctedText.replace(index, 1, "\\P");
    }

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

    // Drawing direction encoding (code 72): 1=LtR, 3=TtB, 5=byStyle.
    // lc::TextConst::DrawingDirection: None=0, Backward=1, UpsideDown=3.
    // Reader's addMText maps 1->Backward, 3->UpsideDown, else->None.  Match
    // that inverse here so round-trip is symmetric.
    // libdxfrw types alignH/alignV as enums (DRW_Text::HAlign/VAlign) even on
    // the DRW_MText path where the DXF semantics have nothing to do with
    // TEXT alignment.  Cast is required — the integer we set here is what the
    // writer emits verbatim as code 72 / code 73 for MTEXT.
    switch (t->textgeneration()) {
        case lc::TextConst::DrawingDirection::Backward:
            tex.alignH = static_cast<DRW_Text::HAlign>(1); break;
        case lc::TextConst::DrawingDirection::UpsideDown:
            tex.alignH = static_cast<DRW_Text::HAlign>(3); break;
        default:
            tex.alignH = static_cast<DRW_Text::HAlign>(0); break;
    }
    // Line spacing style (code 73): 1=at least, 2=exact.  lc::entity::MText
    // doesn't currently carry a spacing style so default to 1.
    tex.alignV = static_cast<DRW_Text::VAlign>(1);

    dxfW->writeMText(&tex);
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
}

void DXFimpl::writeBlockRecords() {
    for(const auto& block : _document->blocks()) {
        dxfW->writeBlockRecord(block->name());
    }
}

void DXFimpl::writeBlocks() {
    for(const auto& block : _document->blocks()) {
        writeBlock(block);
    }
}

void DXFimpl::writeBlock(const lc::meta::Block_CSPtr& block) {
    DRW_Block drwBlock;

    drwBlock.name = block->name();
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
