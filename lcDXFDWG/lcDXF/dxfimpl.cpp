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

    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    if (data.color == 256 && data.lWeight == 29) {
        _builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(
                                                        data.basePoint.x,
                                                        data.basePoint.y),
                                                    lc::geo::Coordinate(
                                                        data.secPoint.x,
                                                        data.secPoint.y),l));
    } else {
        auto mf = lc::MetaInfo::create();
        if ( data.color > 256) {
            auto mc = std::make_shared<lc::MetaColor>(col);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
        }
        if ( data.lWeight < 24) {
            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
            auto lw = std::make_shared<lc::MetaLineWidth>(len);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
        }
        _builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(
                                                        data.basePoint.x,
                                                        data.basePoint.y),
                                                    lc::geo::Coordinate(
                                                        data.secPoint.x,
                                                        data.secPoint.y),l, mf));
    }
}

void DXFimpl::addCircle(const DRW_Circle& data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);

    if (data.color == 256 && data.lWeight == 29) {
        _builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                                                          data.basePoint.x,
                                                          data.basePoint.y),
                                                      data.radious, l));
    } else {
        auto mf = lc::MetaInfo::create();
        if ( data.color > 256) {
            auto mc = std::make_shared<lc::MetaColor>(col);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
        }
        if ( data.lWeight < 24) {
            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
            auto lw = std::make_shared<lc::MetaLineWidth>(len);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
        }

        _builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                                                          data.basePoint.x,
                                                          data.basePoint.y),
                                                      data.radious, l, mf));

    }
}

void DXFimpl::addArc(const DRW_Arc &data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    if (data.color == 256 && data.lWeight == 29) {
        _builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(
                                                       data.basePoint.x,
                                                       data.basePoint.y),
                                                   data.radious,
                                                   data.staangle, data.endangle,
                                                   l));
    } else {
        auto mf = lc::MetaInfo::create();
        if ( data.color > 256) {
            auto mc = std::make_shared<lc::MetaColor>(col);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
        }
        if ( data.lWeight  < 24 ) {
            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
            auto lw = std::make_shared<lc::MetaLineWidth>(len);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
        }
        _builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                                                          data.basePoint.x,
                                                          data.basePoint.y),
                                                      data.radious, l, mf));
    }
}

void DXFimpl::addEllipse(const DRW_Ellipse &data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    if (data.color == 256 && data.lWeight == 29) {
        _builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(
                                                           data.basePoint.x,
                                                           data.basePoint.y),
                                                       lc::geo::Coordinate(data.secPoint.x, data.secPoint.y),
                                                       lc::geo::Coordinate(data.basePoint.x,data.basePoint.y).distanceTo(lc::geo::Coordinate(data.secPoint.x, data.secPoint.y))/data.ratio,
                                                       data.staparam, data.endparam,
                                                       l));
    } else {
        auto mf = lc::MetaInfo::create();
        if ( data.color > 256 ) {
            auto mc = std::make_shared<lc::MetaColor>(col);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
        }
        if ( data.lWeight < 24) {
            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
            auto lw = std::make_shared<lc::MetaLineWidth>(len);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
        }
        _builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(
                                                           data.basePoint.x,
                                                           data.basePoint.y),
                                                       lc::geo::Coordinate(data.secPoint.x, data.secPoint.y),
                                                       lc::geo::Coordinate(data.basePoint.x,data.basePoint.y).distanceTo(lc::geo::Coordinate(data.secPoint.x, data.secPoint.y))/data.ratio,
                                                       data.staparam, data.endparam,
                                                       l, mf));
    }

}
void DXFimpl::addLayer(const DRW_Layer &data) {
    auto col = icol.intToColor(data.color);
    auto layer=std::make_shared<lc::Layer>(data.name, data.lWeight, col);
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();
}

void DXFimpl::addSpline(const DRW_Spline& data) {

}

void DXFimpl::addText(const DRW_Text& data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    if (data.color == 256 && data.lWeight == 29) {
        _builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(
                                                        data.basePoint.x,
                                                        data.basePoint.y),
                                                    data.text, data.height,
                                                    data.angle, data.style,
                                                    lc::TextConst::DrawingDirection(data.textgen),
                                                    lc::TextConst::HAlign(data.alignH),
                                                    lc::TextConst::VAlign(data.alignV),
                                                    l));
    } else {
        auto mf = lc::MetaInfo::create();
        if ( data.color > 256 ) {
            auto mc = std::make_shared<lc::MetaColor>(col);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
        }
        if ( data.lWeight < 24) {
            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
            auto lw = std::make_shared<lc::MetaLineWidth>(len);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
        }
        _builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(
                                                        data.basePoint.x,
                                                        data.basePoint.y),
                                                    data.text, data.height,
                                                    data.angle, data.style,
                                                    lc::TextConst::DrawingDirection(data.textgen),
                                                    lc::TextConst::HAlign(data.alignH),
                                                    lc::TextConst::VAlign(data.alignV),
                                                    l, mf));

    }

}

void DXFimpl::addPoint(const DRW_Point& data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    if (data.color == 256 && data.lWeight == 29) {
        _builder->append(std::make_shared<lc::Coordinate>(
                             data.basePoint.x,
                             data.basePoint.y,
                             l));
    } else {
        auto mf = lc::MetaInfo::create();
        if ( data.color > 256 ) {
            auto mc = std::make_shared<lc::MetaColor>(col);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(mc));
        }
        if ( data.lWeight < 24) {
            int len = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
            auto lw = std::make_shared<lc::MetaLineWidth>(len);
            mf->add(std::dynamic_pointer_cast<lc::MetaType>(lw));
        }
        _builder->append(std::make_shared<lc::Coordinate>(
                             data.basePoint.x,
                             data.basePoint.y,
                             l, mf));

    }
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

void DXFimpl::addDimAngular3P(const DRW_DimAngular3p* data) {}
void DXFimpl::addDimOrdinate(const DRW_DimOrdinate* data) {}
void DXFimpl::addLWPolyline(const DRW_LWPolyline& data) {}
void DXFimpl::addPolyline(const DRW_Polyline& data) {}
void DXFimpl::addSpline(const DRW_Spline* data) {}


void DXFimpl::addMText(const DRW_MText& data) {}
void DXFimpl::addHatch(const DRW_Hatch* data) {}
