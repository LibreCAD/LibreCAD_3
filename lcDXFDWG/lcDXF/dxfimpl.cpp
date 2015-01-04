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

DXFimpl::DXFimpl(lc::Document * d, lc::operation::Builder_SPtr builder) : _document(d), _builder(builder) {
}
void DXFimpl::addLine(const DRW_Line& data) {

    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(
                                                   data.basePoint.x,
                                                   data.basePoint.y),
                                               lc::geo::Coordinate(
                                                   data.secPoint.x,
                                                   data.secPoint.y),l));
}

void DXFimpl::addCircle(const DRW_Circle& data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                                                     data.basePoint.x,
                                                     data.basePoint.y),
                                                 data.radious, l));
}

void DXFimpl::addArc(const DRW_Arc &data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(
                                                  data.basePoint.x,
                                                  data.basePoint.y),
                                              data.radious,
                                              data.staangle, data.endangle,
                                              l));
}

void DXFimpl::addEllipse(const DRW_Ellipse &data) {
    auto col = icol.intToColor(data.color);
    auto l = _document->layerByName(data.layer);
    _builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(
                                                      data.basePoint.x,
                                                      data.basePoint.y),
                                                  lc::geo::Coordinate(data.secPoint.x, data.secPoint.y),
                                                  lc::geo::Coordinate(data.basePoint.x,data.basePoint.y).distanceTo(lc::geo::Coordinate(data.secPoint.x, data.secPoint.y))/data.ratio,
                                                  data.staparam, data.endparam,
                                                  l));
}


void DXFimpl::addLayer(const DRW_Layer &data) {
    auto col = icol.intToColor(data.color);
    auto layer=std::make_shared<lc::Layer>(data.name, data.lWeight, col);
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();
}
