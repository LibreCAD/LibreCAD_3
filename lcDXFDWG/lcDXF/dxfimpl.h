#pragma once

#include "dxfrw_intf.h"

#include <cad/document/document.h>
#include <cad/document/storagemanager.h>
#include <cad/meta/icolor.h>
#include <cad/operations/builder.h>


static const char *const SKIP_BYLAYER = "BYLAYER";
static const char *const SKIP_CONTINUOUS = "CONTINUOUS";

class DXFimpl : public DRW_InterfaceImpl {
    public:
        DXFimpl(std::shared_ptr<lc::Document> document, lc::operation::Builder_SPtr builder);
        virtual void addLine(const DRW_Line& data) override;
        virtual void addCircle(const DRW_Circle& data) override;
        virtual void addLayer(const DRW_Layer& data) override;
        virtual void addArc(const DRW_Arc& data) override;
        virtual void addEllipse(const DRW_Ellipse& data) override;
        virtual void addText(const DRW_Text& data) override;
        virtual void addDimAlign(const DRW_DimAligned* data) override;
        virtual void addDimLinear(const DRW_DimLinear* data) override;
        virtual void addDimRadial(const DRW_DimRadial* data) override;
        virtual void addDimDiametric(const DRW_DimDiametric* data) override;
        virtual void addDimAngular(const DRW_DimAngular* data) override;
        virtual void addDimAngular3P(const DRW_DimAngular3p* data) override;
        virtual void addDimOrdinate(const DRW_DimOrdinate* data) override;
        virtual void addLWPolyline(const DRW_LWPolyline& data) override;
        virtual void addPolyline(const DRW_Polyline& data) override;
        virtual void addSpline(const DRW_Spline* data) override;
        virtual void addPoint(const DRW_Point& data) override;
        virtual void addMText(const DRW_MText& data) override;
        virtual void addHatch(const DRW_Hatch* data) override;
        virtual void setBlock(const int handle) override;
        virtual void addBlock(const DRW_Block& data) override;
        virtual void endBlock() override;
        virtual void addLType(const DRW_LType& data) override;

        template <typename T>
        std::shared_ptr<const T> getLcLineWidth(DRW_LW_Conv::lineWidth lw) const {
            std::shared_ptr<const T> mlw = nullptr;
            lc::MetaType_CSPtr tmp = nullptr;
            switch (lw) {
                case DRW_LW_Conv::lineWidth::widthDefault:
                    // By layer is always the default
                    break;
                case DRW_LW_Conv::lineWidth::widthByLayer:
                    // By layer is always the default
                    break;
                case DRW_LW_Conv::lineWidth::widthByBlock:
                    tmp = std::make_shared<lc::MetaLineWidthByBlock>();
                    mlw=std::dynamic_pointer_cast<const T>(tmp);
                default:
                    if (lw >= DRW_LW_Conv::lineWidth::width00 && lw <= DRW_LW_Conv::lineWidth::width23) {
                        mlw=DXFimpl::_intToLineWidth[lw];
                    }
            }

            return mlw;
        }

        std::shared_ptr<lc::Document> _document;
        lc::operation::Builder_SPtr _builder;
        int _blockHandle;


    private:
        /**
        * Return the MetaInfo object from a DRW_Entity.
        * This is usefull because most/all entities will share teh same basic properties
        * can be be read with just one routine
        */
        lc::MetaInfo_SPtr getMetaInfo(DRW_Entity const&) const;
        /**
        * Convert from a DRW_Coord to a geo::Coordinate
        */
        lc::geo::Coordinate coord(DRW_Coord const& coord) const;
        std::vector<lc::geo::Coordinate>coords(std::vector<DRW_Coord *> coordList) const;
    private:
        lc::iColor icol;

        std::shared_ptr<lc::MetaLineWidthByValue> _intToLineWidth[24];


};


