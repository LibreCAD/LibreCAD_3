#pragma once

#include <libdxfrw.h>
#include <drw_interface.h>
#include <drw_base.h>
#include <iostream>
#include "../file.h"
#include "../format.h"
#include "../readguard.h"
#include "preservedrecords.h"
#include "../generic/helpers.h"

#include <cad/storage/document.h>
#include <cad/storage/storagemanager.h>

#include <cad/meta/icolor.h>
#include <cad/operations/entitybuilder.h>
#include <cad/base/visitor.h>
#include <cad/meta/dxflinepattern.h>
#include <cad/meta/metalinewidth.h>
#include <cad/meta/metacolor.h>
#include <cad/base/metainfo.h>
#include <cad/meta/icolor.h>
#include <tuple>
#include <cad/meta/block.h>
#include <cad/operations/builder.h>

#define BYBLOCK_COLOR 0
#define LTYPE_BYBLOCK "ByBlock"

#define APP_NAME_CODE 102
#define PLUGIN_NAME_CODE 410
#define ENTITY_NAME_CODE 411
#define APP_NAME "LibreCAD"
#define DEFAULT_VIEWPORT "*Model_Space"

static const char *const SKIP_BYLAYER = "BYLAYER";
static const char *const SKIP_CONTINUOUS = "CONTINUOUS";

namespace lc {
namespace persistence {

/**
 * The header variables LibreCAD reads from a drawing, captured in one pass by
 * DXFimpl::addHeader.
 *
 * These live here rather than on the document because lckernel has no notion
 * of a drawing header: it has no units, no extents and no global line-type
 * scale.  Keeping them on the importer means the information survives the read
 * -- File::open already uses acadVersion to decide the recorded variant -- and
 * gives the phase 2 ImportResult something to carry instead of re-reading the
 * file.
 */
struct DrawingHeader {
    /** $ACADVER exactly as the drawing writes it; empty when it carries none. */
    std::string acadVersion;

    /** $INSUNITS as the raw DXF code, and as the kernel's enum. */
    int insUnitsCode{0};
    lc::Units units{lc::Units::None};

    /** $MEASUREMENT: 0 = imperial (English), 1 = metric. */
    int measurement{0};

    /** $LTSCALE, the global line-type scale. */
    double lineTypeScale{1.0};

    /** $EXTMIN/$EXTMAX. Both are present or neither is. */
    bool hasExtents{false};
    lc::geo::Coordinate extMin;
    lc::geo::Coordinate extMax;
};

class DXFimpl : public DRW_Interface {
public:

    DXFimpl(std::shared_ptr<lc::storage::Document> document, lc::operation::Builder_SPtr builder);

    DXFimpl(std::shared_ptr<lc::storage::Document> document) : _document(document) {}

    /**
     * Tell the reader the file is a DWG.
     *
     * The DWG reader never fills in DRW_Block::flags, so a DWG's anonymous
     * blocks arrive looking exactly like user blocks. Saying so up front lets
     * addBlock fall back to the *D<n> naming convention for that one source,
     * without loosening the test for DXF, where the flag is real.
     */
    void setSourceIsDwg(bool isDwg) {
        _sourceIsDwg = isDwg;
    }

    // READ FUNCTIONALITY
    void addHeader(const DRW_Header* data) override;


    /** The entities this read had to give up on, one entry each. */
    const std::vector<ImportFailure>& failures() const {
        return _failures;
    }

    /**
     * What this read could not carry: one entry per DXF record kind LibreCAD
     * has no entity for. These callbacks have always been empty overrides; the
     * only change is that the drop is now counted rather than invisible.
     */
    const LossSummary& loss() const {
        return _loss;
    }

    /** How many entities the reader handed to the document. */
    std::size_t entitiesDelivered() const {
        return _entitiesDelivered;
    }

    /**
     * What addHeader captured. Empty/defaulted when the drawing has no HEADER
     * section, which is legal DXF and which three files in the review corpus
     * do.
     */
    const DrawingHeader& header() const {
        return _header;
    }

    // Not counted as loss. Every conforming DXF carries a DIMSTYLE table with
    // at least the default "Standard" entry -- including every file this
    // application writes -- so counting it made the "records that were not
    // imported" dialog fire on essentially every open, reporting loss where
    // there was none and devaluing the dialog for the records it exists for.
    // LibreCAD still has no dimension-style model; the honest place to say so
    // is a note, not a per-open modal.
    void addDimStyle(const DRW_Dimstyle& data) override {
        (void)data;
    }

    void addVport(const DRW_Vport& data) override;

    // Not counted as loss, for the same reason as addDimStyle above: a STYLE
    // table with a default "Standard" entry is boilerplate every DXF carries.
    // LibreCAD has no text style model -- a drawing's fonts are not represented
    // anywhere it could put them -- but that is a standing limitation, not
    // something a particular file lost.
    void addTextStyle(const DRW_Textstyle& data) override {
        (void)data;
    }

    void addAppId(const DRW_AppId& data) override {}

    void addRay(const DRW_Ray& data) override {
        recordLoss("RAY");
    }

    void addXline(const DRW_Xline& data) override {
        recordLoss("XLINE");
    }

    void addKnot(const DRW_Entity& data) override {}

    void addInsert(const DRW_Insert& data) override;

    void addTrace(const DRW_Trace& data) override {
        recordLoss("TRACE");
    }

    void add3dFace(const DRW_3Dface& data) override {
        recordLoss("3DFACE");
    }

    void addSolid(const DRW_Solid& data) override;

    void addLeader(const DRW_Leader* data) override {
        recordLoss("LEADER");
    }

    void addViewport(const DRW_Viewport& data) override;

    void linkImage(const DRW_ImageDef* data) override;

    void addComment(const char* comment) override {}

    // Lossless passthrough. libdxfrw hands over every OBJECTS record, entity
    // and whole section it does not model, plus the CLASSES entries that make
    // them readable again. LibreCAD dropped all of it, so opening a drawing and
    // saving it destroyed everything the file carried beyond the geometry --
    // layouts, plot settings, table styles, dictionaries, another
    // application's custom data -- while the drawing still looked right.
    void addRawDxfObject(const DRW_RawDxfObject& data) override {
        _preserved.objects.push_back(data);
    }

    void addRawDxfEntity(const DRW_RawDxfObject& data) override {
        _preserved.entities.push_back(data);
    }

    void addRawDxfSection(const DRW_RawDxfSection& data) override {
        _preserved.sections.push_back(data);
    }

    void addDxfClass(const DRW_Class& data) override {
        _preserved.classes.push_back(data);
    }

    /** What this read is holding on to for the next save. */
    const PreservedRecords& preserved() const {
        return _preserved;
    }

    /**
     * Hand the preserved records to the document, which is the only thing that
     * survives from this read to the next save: File::save builds a fresh
     * writer from a document and knows nothing about where it came from.
     */
    void attachPreservedRecords();

    void addLine(const DRW_Line& data) override;

    void addCircle(const DRW_Circle& data) override;

    void addLayer(const DRW_Layer& data) override;

    void addArc(const DRW_Arc& data) override;

    void addEllipse(const DRW_Ellipse& data) override;

    void addText(const DRW_Text& data) override;

    void addMText(const DRW_MText& data) override;

    void addDimAlign(const DRW_DimAligned* data) override;

    void addDimLinear(const DRW_DimLinear* data) override;

    void addDimRadial(const DRW_DimRadial* data) override;

    void addDimDiametric(const DRW_DimDiametric* data) override;

    void addDimAngular(const DRW_DimAngular* data) override;

    void addDimAngular3P(const DRW_DimAngular3p* data) override;

    void addDimOrdinate(const DRW_DimOrdinate* data) override;

    void addLWPolyline(const DRW_LWPolyline& data) override;

    void addPolyline(const DRW_Polyline& data) override;

    void addSpline(const DRW_Spline* data) override;

    void addPoint(const DRW_Point& data) override;

    void addHatch(const DRW_Hatch* data) override;

    void addBlock(const DRW_Block& data) override;

    void addLType(const DRW_LType& data) override;

    void addImage(const DRW_Image* data) override;

    void setBlock(const int handle) override;

    void endBlock() override;

    /**
     * Create the INSERT entities held back during the read.
     *
     * An INSERT's bounding box is the union of the boxes of the entities in
     * the block it displays, computed once in Insert's constructor. During a
     * read no entity has reached the document yet -- everything the callbacks
     * build is queued in an operation::Builder that runs afterwards -- so an
     * Insert built as it was read always measured an empty block and came out
     * as a degenerate point at its own insertion point. Deferring the
     * construction until the blocks and their contents are in the document is
     * what makes the box right, and it is also what lets the INSERT reference
     * the block the file actually defined instead of a fabricated stand-in.
     *
     * Called by File::open once the read's own operations have executed.
     */
    void buildDeferredInserts();


    // WRITE FUNCTIONALITY
    bool writeDXF(const std::string& filename, lc::persistence::File::Type type);

    void writeHeader(DRW_Header& data) override;

    void writeBlocks() override;

    void writeBlockRecords() override;

    void writeEntities() override;

    void writeLTypes() override;

    void writeLayers() override;

    void writeTextstyles() override {}

    void writeVports() override {}

    void writeDimstyles() override {}

    void writeAppId() override;

    void writeObjects() override;

    void addPlotSettings(const DRW_PlotSettings *data) override {}

    void getEntityAttributes(DRW_Entity* ent, const lc::entity::CADEntity_CSPtr& entity);

    void writeEntity(const lc::entity::CADEntity_CSPtr& entity);

    void writePoint(const lc::entity::Point_CSPtr& p);

    void writeLine(const lc::entity::Line_CSPtr& l);

    void writeCircle(const lc::entity::Circle_CSPtr& c);

    void writeArc(const lc::entity::Arc_CSPtr& a);

    void writeEllipse(const lc::entity::Ellipse_CSPtr& s);

    void writeSpline(const lc::entity::Spline_CSPtr& s);

    // Issue #412 phase 1: five per-subtype dimension writers plus a shared
    // helper that populates the DRW_Dimension base fields.  Per-subtype
    // dispatch is required because (a) getEntityAttributes() needs the
    // CADEntity (which Dimension itself isn't), (b) each subtype has its
    // own geometry, and (c) DXF group 70 must be set per subtype — otherwise
    // every dimension reads back as linear.
    void writeDimensionCommon(DRW_Dimension* dim,
                              const lc::entity::CADEntity_CSPtr& entity,
                              const lc::entity::Dimension& d);

    void writeDimLinear(const lc::entity::DimLinear_CSPtr& d);
    void writeDimAligned(const lc::entity::DimAligned_CSPtr& d);
    void writeDimRadial(const lc::entity::DimRadial_CSPtr& d);
    void writeDimDiametric(const lc::entity::DimDiametric_CSPtr& d);
    void writeDimAngular(const lc::entity::DimAngular_CSPtr& d);

    void writeLWPolyline(const lc::entity::LWPolyline_CSPtr& p);

    void writeImage(const lc::entity::Image_CSPtr& i);

    // Issue #412 phase 1: previously no writer existed for Hatch — not even
    // a declaration — so hatches vanished from any DXF save.
    void writeHatch(const lc::entity::Hatch_CSPtr& h);

    void writeText(const lc::entity::Text_CSPtr& t);

    void writeMText(const lc::entity::MText_CSPtr& t);

    void writeInsert(const lc::entity::Insert_CSPtr& i);

    void writeLayer(const std::shared_ptr<const lc::meta::Layer>& layer);

    void writeBlock(const lc::meta::Block_CSPtr& block);

    /**
     * Emit the anonymous *D block that holds a dimension's drawn geometry.
     *
     * DXF group 2 on a DIMENSION names a block containing the lines, arrowheads
     * and text that make up the picture. LibreCAD wrote no such block and no
     * group 2, and that is not the harmless omission it looks like: ezdxf's
     * audit **deletes** a DIMENSION that has no valid geometry block, so a
     * drawing saved by LibreCAD lost its dimensions entirely when it passed
     * through anything that audits on load.
     *
     * Returns the block name to put in group 2, or "" when the kind has no
     * geometry this can build.
     */
    std::string writeDimensionBlock(const lc::entity::CADEntity_CSPtr& entity);

    // Pieces shared by the per-kind geometry above. They take plain numbers
    // rather than a geometry type so that this header keeps naming no DRW type
    // beyond the ones DRW_Interface already forces on it.
    std::string beginDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                    std::string& layerName);
    void writeDimensionLine(const std::string& layerName,
                            double fromX, double fromY, double toX, double toY);
    void writeDimensionArrow(const std::string& layerName,
                             double tipX, double tipY, double dirX, double dirY);
    void writeDimensionText(const std::string& layerName,
                            const lc::entity::Dimension& dimension, const std::string& value);

    /** A dimension drawn as a single measured line: radial and diametric. */
    std::string writeLeaderDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                          const lc::entity::Dimension& dimension,
                                          double fromX, double fromY, double toX, double toY,
                                          bool arrowAtBothEnds, const std::string& prefix);

    std::string writeAngularDimensionBlock(const lc::entity::CADEntity_CSPtr& entity,
                                           const lc::entity::Dimension& dimension,
                                           const lc::entity::DimAngular& angular);

    /** Every dimension in the document, in one stable order. */
    std::vector<lc::entity::CADEntity_CSPtr> allDimensions() const;

    /** The *D block name assigned to each dimension, by entity id. */
    std::map<ID_DATATYPE, std::string> _dimensionBlocks;

    /// Whether the document being read came from a DWG. The DWG reader leaves
    /// DRW_Block::flags at zero, so the anonymous bit a DXF carries is simply
    /// not there and the naming convention has to stand in for it.
    bool _sourceIsDwg{false};

    static bool isAnonymousDimensionBlockName(const std::string& name);
    static bool isRegeneratedDimensionBlock(const lc::meta::Block_CSPtr& block);
    unsigned int _nextDimensionBlock{1};


    // UTILITIES FUNCTIONS
    lc::AngleFormat numberToAngleFormat(int num);

    int angleFormatToNumber(lc::AngleFormat af);

    lc::Units numberToUnit(int num);

    int unitToNumber(lc::Units unit);

    inline int widthToInt(double wid) const;

    template<typename T>
    std::shared_ptr<const T> getLcLineWidth(DRW_LW_Conv::lineWidth lw) const {
        std::shared_ptr<const T> mlw = nullptr;
        lc::meta::MetaType_CSPtr tmp = nullptr;
        switch (lw) {
        case DRW_LW_Conv::lineWidth::widthDefault:
            // By layer is always the default
            break;
        case DRW_LW_Conv::lineWidth::widthByLayer:
            // By layer is always the default
            break;
        case DRW_LW_Conv::lineWidth::widthByBlock:
            tmp = std::make_shared<lc::meta::MetaLineWidthByBlock>();
            mlw = std::dynamic_pointer_cast<const T>(tmp);
        default:
            if (lw >= DRW_LW_Conv::lineWidth::width00 && lw <= DRW_LW_Conv::lineWidth::width23) {
                mlw = std::make_shared<lc::meta::MetaLineWidthByValue>(lc::persistence::FileHelpers::intToLW(lw).width());
            }
        }

        return mlw;
    }

    std::shared_ptr<lc::storage::Document> _document;
    lc::operation::Builder_SPtr _builder;
    lc::operation::EntityBuilder_SPtr _entityBuilder;
    lc::meta::Block_SPtr _currentBlock;
    DrawingHeader _header;
    LossSummary _loss;
    PreservedRecords _preserved;
    std::vector<ImportFailure> _failures;
    std::size_t _entitiesDelivered{0};
    std::size_t _attributesAsText{0};

    void recordLoss(const char* recordKind) {
        _loss.droppedByType[recordKind]++;
    }

    /**
     * Run one read callback, and survive it throwing.
     *
     * libdxfrw's read path is not a function-try-block the way its write path
     * is, so an exception from a callback leaves dxfRW::read() with
     * getError() == BAD_NONE: the caller is told the file read cleanly while
     * the process is already unwinding. LibreCAD's own kernel throws from
     * inside these callbacks -- geo::Arc for an unusable radius, geo::Area for
     * a volume -- and one bad record used to end the program.
     *
     * The catch is on `...`, not on std::exception: geoarea.h threw a bare
     * `const char*` for years, which a std::exception handler walks straight
     * past.
     *
     * This could not land before the crashes were fixed. It converts a
     * reproducible abort into a recorded skip, which is only an improvement
     * once the aborts are understood -- otherwise it deletes the signal that
     * found them.
     */
    template<typename Body>
    void guarded(const char* recordKind, const DRW_Entity* entity, Body body) {
        std::string reason;
        if (!runGuarded(body, reason)) {
            recordFailure(recordKind, entity, reason.c_str());
        }
    }

    /**
     * Hand one entity to the document, and count it.
     *
     * The count is of what the *reader* produced, which is not the same as what
     * the document ends up holding: a block's contents live in the block, not
     * the entity container. Importers report the former.
     */
    void deliver(const lc::entity::CADEntity_CSPtr& entity) {
        _entitiesDelivered++;
        _entityBuilder->appendEntity(entity);
    }

    /** An INSERT read from the file, not yet turned into an entity. */
    struct PendingInsert {
        lc::meta::MetaInfo_SPtr metaInfo;
        lc::meta::Block_CSPtr containerBlock;  //!< the block the INSERT sits in, null for model space
        lc::meta::Layer_CSPtr layer;
        lc::geo::Coordinate position;
        std::string targetBlockName;           //!< the block the INSERT displays
    };
    std::vector<PendingInsert> _pendingInserts;

private:
    void recordFailure(const char* recordKind, const DRW_Entity* entity, const char* reason);

    /**
    * Return the MetaInfo object from a DRW_Entity.
    * This is useful because most/all entities will share the same basic properties
    * can be be read with just one routine
    */

    dxfRW* dxfW;

    /**
     * The DXF revision the current write targets. R12 cannot carry several
     * entity kinds, so the writers need to know what they are writing for.
     */
    DRW::Version _exportVersion{DRW::AC1024};

    /** The preserved records being replayed by the current write, if any. */
    std::shared_ptr<const PreservedRecords> _replay;

    /** The target of the current write, for the format-capability check. */
    lc::persistence::File::Type _exportType{lc::persistence::File::LIBDXFRW_DXF_R2010};

    lc::meta::MetaInfo_SPtr getMetaInfo(DRW_Entity const& data) const;

    lc::meta::Block_CSPtr getBlock(DRW_Entity const& data) const;

    /**
     * DXF symbol-table names are case-insensitive; writers spell model space
     * both *Model_Space and *MODEL_SPACE.
     */
    static bool isModelSpaceName(const std::string& name);

    // this function adds layer too if not found: issue with some files
    lc::meta::Layer_CSPtr getLayer(DRW_Entity const& data) const;

    /**
    * Convert from a DRW_Coord to a geo::Coordinate
    */
    lc::geo::Coordinate coord(DRW_Coord const& coord) const;

    std::vector<lc::geo::Coordinate> coords(std::vector<std::shared_ptr<DRW_Coord>> coordList) const;

private:
    lc::iColor icol;

    std::vector<DRW_Image> imageMapCache;
    //std::map<std::string, lc::meta::Block_CSPtr> _blocks;
    std::map<int, lc::meta::Block_CSPtr> _handleBlock;

    const static std::map<int, lc::Units> _dxfToLCUnits;
    const static std::map<lc::Units, int> _lcUnitsToDXF;
};
}
}
