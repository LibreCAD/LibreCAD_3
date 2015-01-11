#include "dxfimpl.h"

#include <cad/primitive/circle.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/coordinate.h>
#include <cad/operations/builder.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/meta/color.h>
#include <cad/meta/icolor.h>
#include "cad/base/metainfo.h"
#include "cad/meta/metacolor.h"
#include "cad/meta/metalinewidth.h"

DXFimpl::DXFimpl(lc::Document * d, lc::operation::Builder_SPtr builder) : _document(d), _builder(builder) {
}
void DXFimpl::addLine(const DRW_Line& data) {
    std::shared_ptr<lc::MetaInfo> mf;

    auto lw = getLcLineWidth(data.lWeight);
    if (lw!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(lw);
    }
    auto col = icol.intToColor(data.color);
    if (col!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(col);
    }

    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(
                    data.basePoint.x,
                    data.basePoint.y),
            lc::geo::Coordinate(
                    data.secPoint.x,
                    data.secPoint.y),layer, mf));
}

void DXFimpl::addCircle(const DRW_Circle& data) {
     std::shared_ptr<lc::MetaInfo> mf;

    auto lw = getLcLineWidth(data.lWeight);
    if (lw!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(lw);
    }
    auto col = icol.intToColor(data.color);
    if (col!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(col);
    }

    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                    data.basePoint.x,
                    data.basePoint.y),
            data.radious, layer, mf));

}

void DXFimpl::addArc(const DRW_Arc &data) {
     std::shared_ptr<lc::MetaInfo> mf;

    auto lw = getLcLineWidth(data.lWeight);
    if (lw!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(lw);
    }
    auto col = icol.intToColor(data.color);
    if (col!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(col);
    }

    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                    data.basePoint.x,
                    data.basePoint.y),
            data.radious, layer, mf));
}

void DXFimpl::addEllipse(const DRW_Ellipse &data) {
     std::shared_ptr<lc::MetaInfo> mf;

    auto lw = getLcLineWidth(data.lWeight);
    if (lw!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(lw);
    }
    auto col = icol.intToColor(data.color);
    if (col!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(col);
    }

    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(
                    data.basePoint.x,
                    data.basePoint.y),
            lc::geo::Coordinate(data.secPoint.x, data.secPoint.y),
            lc::geo::Coordinate(data.basePoint.x,data.basePoint.y).distanceTo(lc::geo::Coordinate(data.secPoint.x, data.secPoint.y))/data.ratio,
            data.staparam, data.endparam,
            layer, mf));
}

void DXFimpl::addLayer(const DRW_Layer &data) {
    auto col = icol.intToColor(data.color);
    auto layer=std::make_shared<lc::Layer>(data.name, 0., col->color());
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();
}

void DXFimpl::addSpline(const DRW_Spline& data) {

}

void DXFimpl::addText(const DRW_Text& data) {
     std::shared_ptr<lc::MetaInfo> mf;

    auto lw = getLcLineWidth(data.lWeight);
    if (lw!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(lw);
    }
    auto col = icol.intToColor(data.color);
    if (col!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(col);
    }

    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(
                    data.basePoint.x,
                    data.basePoint.y),
            data.text, data.height,
            data.angle, data.style,
            lc::TextConst::DrawingDirection(data.textgen),
            lc::TextConst::HAlign(data.alignH),
            lc::TextConst::VAlign(data.alignV),
            layer, mf));
}

void DXFimpl::addPoint(const DRW_Point& data) {
     std::shared_ptr<lc::MetaInfo> mf;

    auto lw = getLcLineWidth(data.lWeight);
    if (lw!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(lw);
    }
    auto col = icol.intToColor(data.color);
    if (col!=nullptr) {
        if (mf==nullptr) lc::MetaInfo::create();
        mf->add(col);
    }

    auto layer = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Coordinate>(
            data.basePoint.x,
            data.basePoint.y,
            layer, mf));
}

void DXFimpl::addDimAlign(const DRW_DimAligned* data) {
//    lc::Dimension dim(data->getDefPoint(), data->getTextPoint(), data->getAlign(),
//                      data->getTextLineStyle(), data->getTextLineFactor(),
//                      data->getText(), data->getStyle(), data->getAngle());

//    auto col = icol.intToColor(data.color);
//    auto l = _document->layerByName(data.layer);
//    if (data.color == 256 && data.lWeight == 29) {
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             dim, data->getDef1Point(),
//                             data->getDef2Point(),
//                             l));
//    } else {
//        auto mf = lc::MetaInfo::create();
//        if ( data.color > 256 ) {
//            auto mc = std::make_shared<lc::MetaColor>(col);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
//        }
//        if ( data.lWeight < 24) {
//            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
//            auto lw = std::make_shared<lc::MetaLineWidth>(len);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
//        }
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             dim, data->getDef1Point(),
//                             data->getDef2Point(),
//                             l, mf));

//    }
}

void DXFimpl::addDimLinear(const DRW_DimLinear* data) {
//    lc::Dimension dim(data->getDefPoint(), data->getTextPoint(), data->getAlign(),
//                      data->getTextLineStyle(), data->getTextLineFactor(),
//                      data->getText(), data->getStyle(), data->getAngle());

//    auto col = icol.intToColor(data.color);
//    auto l = _document->layerByName(data.layer);
//    if (data.color == 256 && data.lWeight == 29) {
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l));
//    } else {
//        auto mf = lc::MetaInfo::create();
//        if ( data.color > 256 ) {
//            auto mc = std::make_shared<lc::MetaColor>(col);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
//        }
//        if ( data.lWeight < 24) {
//            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
//            auto lw = std::make_shared<lc::MetaLineWidth>(len);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
//        }
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l, mf));

//    }

}

void DXFimpl::addDimRadial(const DRW_DimRadial* data) {
//    lc::Dimension dim(data->getDefPoint(), data->getTextPoint(), data->getAlign(),
//                      data->getTextLineStyle(), data->getTextLineFactor(),
//                      data->getText(), data->getStyle(), data->getAngle());

//    auto col = icol.intToColor(data.color);
//    auto l = _document->layerByName(data.layer);
//    if (data.color == 256 && data.lWeight == 29) {
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l));
//    } else {
//        auto mf = lc::MetaInfo::create();
//        if ( data.color > 256 ) {
//            auto mc = std::make_shared<lc::MetaColor>(col);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
//        }
//        if ( data.lWeight < 24) {
//            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
//            auto lw = std::make_shared<lc::MetaLineWidth>(len);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
//        }
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l, mf));

//    }
}
void DXFimpl::addDimDiametric(const DRW_DimDiametric* data) {
//    lc::Dimension dim(data->getDefPoint(), data->getTextPoint(), data->getAlign(),
//                      data->getTextLineStyle(), data->getTextLineFactor(),
//                      data->getText(), data->getStyle(), data->getAngle());
//    auto col = icol.intToColor(data.color);
//    auto l = _document->layerByName(data.layer);
//    if (data.color == 256 && data.lWeight == 29) {
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l));
//    } else {
//        auto mf = lc::MetaInfo::create();
//        if ( data.color > 256 ) {
//            auto mc = std::make_shared<lc::MetaColor>(col);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
//        }
//        if ( data.lWeight < 24) {
//            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
//            auto lw = std::make_shared<lc::MetaLineWidth>(len);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
//        }
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l, mf));

//    }
}

void DXFimpl::addDimAngular(const DRW_DimAngular* data) {
//    lc::Dimension dim(data->getDefPoint(), data->getTextPoint(), data->getAlign(),
//                      data->getTextLineStyle(), data->getTextLineFactor(),
//                      data->getText(), data->getStyle(), data->getAngle());

//    auto col = icol.intToColor(data.color);
//    auto l = _document->layerByName(data.layer);
//    if (data.color == 256 && data.lWeight == 29) {
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l));
//    } else {
//        auto mf = lc::MetaInfo::create();
//        if ( data.color > 256 ) {
//            auto mc = std::make_shared<lc::MetaColor>(col);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
//        }
//        if ( data.lWeight < 24) {
//            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
//            auto lw = std::make_shared<lc::MetaLineWidth>(len);
//            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
//        }
//        _builder->append(std::make_shared<lc::DimAligned>(
//                             data.basePoint.x,
//                             data.basePoint.y,
//                             l, mf));

//    }
}

std::shared_ptr<lc::MetaLineWidth> DXFimpl::getLcLineWidth(DRW_LW_Conv::lineWidth lw) const {
    std::shared_ptr<lc::MetaLineWidth> mlw=nullptr;
    switch (lw) {
        case DRW_LW_Conv::lineWidth::widthDefault:
        case DRW_LW_Conv::lineWidth::widthByLayer:
        case DRW_LW_Conv::lineWidth::widthByBlock: // TODO understand how 'by block' works
            return nullptr;
        default:
            if (lw>=0 && lw < 24) {
                mlw = DXFimpl::_intToLineWidth[lw];
            }
    }
    return mlw;
}


void DXFimpl::addDimAngular3P(const DRW_DimAngular3p* data) {}
void DXFimpl::addDimOrdinate(const DRW_DimOrdinate* data) {}
void DXFimpl::addLWPolyline(const DRW_LWPolyline& data) {}
void DXFimpl::addPolyline(const DRW_Polyline& data) {}
void DXFimpl::addSpline(const DRW_Spline* data) {}


void DXFimpl::addMText(const DRW_MText& data) {}
void DXFimpl::addHatch(const DRW_Hatch* data) {}

const std::shared_ptr<lc::MetaLineWidth> DXFimpl::_intToLineWidth[] = {
        std::make_shared<lc::MetaLineWidth>(0.00), 
        std::make_shared<lc::MetaLineWidth>(0.05), 
        std::make_shared<lc::MetaLineWidth>(0.09), 
        std::make_shared<lc::MetaLineWidth>(0.13), 
        std::make_shared<lc::MetaLineWidth>(0.15), 
        std::make_shared<lc::MetaLineWidth>(0.18), 
        std::make_shared<lc::MetaLineWidth>(0.20), 
        std::make_shared<lc::MetaLineWidth>(0.25), 
        std::make_shared<lc::MetaLineWidth>(0.30), 
        std::make_shared<lc::MetaLineWidth>(0.35), 
        std::make_shared<lc::MetaLineWidth>(0.40), 
        std::make_shared<lc::MetaLineWidth>(0.50), 
        std::make_shared<lc::MetaLineWidth>(0.53), 
        std::make_shared<lc::MetaLineWidth>(0.60), 
        std::make_shared<lc::MetaLineWidth>(0.70), 
        std::make_shared<lc::MetaLineWidth>(0.80), 
        std::make_shared<lc::MetaLineWidth>(0.90), 
        std::make_shared<lc::MetaLineWidth>(1.00), 
        std::make_shared<lc::MetaLineWidth>(1.06), 
        std::make_shared<lc::MetaLineWidth>(1.20), 
        std::make_shared<lc::MetaLineWidth>(1.40), 
        std::make_shared<lc::MetaLineWidth>(1.58), 
        std::make_shared<lc::MetaLineWidth>(2.0), 
        std::make_shared<lc::MetaLineWidth>(2.11)
};
