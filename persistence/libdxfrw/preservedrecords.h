#pragma once

#include <string>
#include <vector>

#include <drw_base.h>
#include <drw_classes.h>
#include <drw_objects.h>

#include <cad/interface/metatype.h>

namespace lc {
namespace persistence {

/**
 * The parts of a DXF file LibreCAD does not model, kept verbatim so that saving
 * can put them back.
 *
 * A DXF holds far more than geometry: layouts, plot settings, table styles,
 * dictionaries, and whatever a vertical application stored under its own class.
 * LibreCAD reads none of it and wrote none of it back, so opening a drawing and
 * pressing Save silently destroyed everything the file carried beyond the lines
 * -- and the drawing still looked right, which is what made it dangerous.
 *
 * libdxfrw hands these over verbatim (addRawDxfObject, addRawDxfEntity,
 * addRawDxfSection, addDxfClass) and takes them back the same way. This rides
 * on the document, because that is the only thing that survives from the read
 * to the write: File::save builds a fresh writer from a document and knows
 * nothing about where it came from.
 *
 * It is a DocumentMetaType, which the kernel stores by id without knowing or
 * caring what is in it -- so this needs no lckernel change and no DRW type in
 * any public persistence header. It lives under persistence/libdxfrw/ with the
 * rest of the adapter, where naming DRW types is the job.
 */
class PreservedRecords : public lc::meta::DocumentMetaType {
public:
    /** The key the document files this under. Leading underscore: not a name a
     *  user's layer, block or line type can collide with. */
    static const char* const kId;

    const std::string id() const override {
        return kId;
    }

    const std::string name() const override {
        return kId;
    }

    bool empty() const {
        return objects.empty() && entities.empty() && sections.empty();
    }

    std::size_t total() const {
        return objects.size() + entities.size() + sections.size();
    }

    /** OBJECTS-section records libdxfrw does not model. */
    std::vector<DRW_RawDxfObject> objects;

    /** ENTITIES/BLOCKS records libdxfrw does not model. */
    std::vector<DRW_RawDxfObject> entities;

    /** Whole sections libdxfrw does not model. */
    std::vector<DRW_RawDxfSection> sections;

    /** CLASSES entries, needed for the records above to be readable again. */
    std::vector<DRW_Class> classes;

    /**
     * Header variables the drawing set and LibreCAD cannot otherwise remember.
     *
     * lckernel has no units, no global line-type scale and no measurement
     * system, so a drawing in inches was read, written back as millimetres and
     * silently rescaled by whoever opened it next. These travel with the
     * document for the same reason the records above do, and unlike them they
     * are revision independent -- a drawing's units do not change because it is
     * saved as an older DXF.
     */
    bool hasHeader{false};
    int insUnitsCode{0};
    int measurement{0};
    double lineTypeScale{1.0};

    /**
     * The revision these came from.
     *
     * Replay is only safe into the same revision: a record valid in R2013 has
     * no defined meaning in R12, and re-emitting it there would produce a file
     * that claims to be R12 and is not.
     */
    DRW::Version version{DRW::UNKNOWNV};
};

}  // namespace persistence
}  // namespace lc
