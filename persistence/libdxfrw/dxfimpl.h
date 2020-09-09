#pragma once

#include <libdxfrw.h>
#include <drw_interface.h>
#include <drw_base.h>
#include <iostream>
#include "../file.h"
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
class DXFimpl : public DRW_Interface {
public:

    DXFimpl(std::shared_ptr<lc::storage::Document> document, lc::operation::Builder_SPtr builder);

    DXFimpl(std::shared_ptr<lc::storage::Document> document) : _document(document) {}

    // READ FUNCTIONALITY
    void addHeader(const DRW_Header* data) override {}

    void addDimStyle(const DRW_Dimstyle& data) override {}

    void addVport(const DRW_Vport& data) override;

    void addTextStyle(const DRW_Textstyle& data) override {}

    void addAppId(const DRW_AppId& data) override {}

    void addRay(const DRW_Ray& data) override {}

    void addXline(const DRW_Xline& data) override {}

    void addKnot(const DRW_Entity& data) override {}

    void addInsert(const DRW_Insert& data) override;

    void addTrace(const DRW_Trace& data) override {}

    void add3dFace(const DRW_3Dface& data) override {}

    void addSolid(const DRW_Solid& data) override {}

    void addLeader(const DRW_Leader* data) override {}

    void addViewport(const DRW_Viewport& data) override;

    void linkImage(const DRW_ImageDef* data) override;

    void addComment(const char* comment) override {}

    void addLine(const DRW_Line& data) override;

    void addCircle(const DRW_Circle& data) override;

    void addLayer(const DRW_Layer& data) override;

    void addArc(const DRW_Arc& data) override;

    void addEllipse(const DRW_Ellipse& data) override;

    void addText(const DRW_Text& data) override;

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

    void addMText(const DRW_MText& data) override;

    void addHatch(const DRW_Hatch* data) override;

    void addBlock(const DRW_Block& data) override;

    void addLType(const DRW_LType& data) override;

    void addImage(const DRW_Image* data) override;

    void setBlock(const int handle) override;

    void endBlock() override;


    // WRITE FUNCTIONALITY
    bool writeDXF(const std::string& filename, lc::persistence::File::Type type);

    void writeHeader(DRW_Header& data) override {}

    void writeBlocks() override;

    void writeBlockRecords() override;

    void writeEntities() override;

    void writeLTypes() override;

    void writeLayers() override;

    void writeTextstyles() override {}

    void writeVports() override {}

    void writeDimstyles() override {}

    void writeAppId() override;

    void getEntityAttributes(DRW_Entity* ent, const lc::entity::CADEntity_CSPtr& entity);

    void writeEntity(const lc::entity::CADEntity_CSPtr& entity);

    void writePoint(const lc::entity::Point_CSPtr& p);

    void writeLine(const lc::entity::Line_CSPtr& l);

    void writeCircle(const lc::entity::Circle_CSPtr& c);

    void writeArc(const lc::entity::Arc_CSPtr& a);

    void writeEllipse(const lc::entity::Ellipse_CSPtr& s);

    void writeSpline(const lc::entity::Spline_CSPtr& s);

    void writeDimension(const lc::entity::Dimension_CSPtr& d);

    void writeLWPolyline(const lc::entity::LWPolyline_CSPtr& p);

    void writeImage(const lc::entity::Image_CSPtr& i);

    void writeText(const lc::entity::Text_CSPtr& t);

    void writeInsert(const lc::entity::Insert_CSPtr& i);

    void writeLayer(const std::shared_ptr<const lc::meta::Layer>& layer);

    void writeBlock(const lc::meta::Block_CSPtr& block);

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

private:
    /**
    * Return the MetaInfo object from a DRW_Entity.
    * This is useful because most/all entities will share the same basic properties
    * can be be read with just one routine
    */

    dxfRW* dxfW;

    lc::meta::MetaInfo_SPtr getMetaInfo(DRW_Entity const& data) const;

    lc::meta::Block_CSPtr getBlock(DRW_Entity const& data) const;

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
