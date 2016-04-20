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
        virtual void addLine(const DRW_Line& data);
        virtual void addCircle(const DRW_Circle& data);
        virtual void addLayer(const DRW_Layer& data);
        virtual void addArc(const DRW_Arc& data);
        virtual void addEllipse(const DRW_Ellipse& data);
        virtual void addText(const DRW_Text& data);
        virtual void addDimAlign(const DRW_DimAligned* data);
        virtual void addDimLinear(const DRW_DimLinear* data);
        virtual void addDimRadial(const DRW_DimRadial* data);
        virtual void addDimDiametric(const DRW_DimDiametric* data);
        virtual void addDimAngular(const DRW_DimAngular* data);
        virtual void addDimAngular3P(const DRW_DimAngular3p* data);
        virtual void addDimOrdinate(const DRW_DimOrdinate* data);
        virtual void addLWPolyline(const DRW_LWPolyline& data);
        virtual void addPolyline(const DRW_Polyline& data);
        virtual void addSpline(const DRW_Spline* data);
        virtual void addPoint(const DRW_Point& data);
        virtual void addMText(const DRW_MText& data);
        virtual void addHatch(const DRW_Hatch* data);
        virtual void setBlock(const int handle);
        virtual void addBlock(const DRW_Block& data);
        virtual void endBlock();
        virtual void addLType(const DRW_LType& data);

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


