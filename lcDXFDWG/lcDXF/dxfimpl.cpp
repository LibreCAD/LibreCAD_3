#include "dxfimpl.h"

DXFimpl::DXFimpl(lc::StorageManager_SPtr s, lc::Document* d) {
    _storageManager = s;
    _document = d;
}
void DXFimpl::addLine(const DRW_Line& data) {

    auto col = icol.intToColor(data.color);
    auto L = std::make_shared<lc::Layer >(data.layer, data.lWeight, col);
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(
                                                   data.basePoint.x,
                                                   data.basePoint.y),
                                               lc::geo::Coordinate(
                                                   data.secPoint.x,
                                                   data.secPoint.y),
                                               L)).push();
    builder->execute();
}

void DXFimpl::addCircle(const DRW_Circle& data) {

    auto col = icol.intToColor(data.color);
    auto L = std::make_shared<lc::Layer >(data.layer, data.lWeight, col);
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                                                     data.basePoint.x,
                                                     data.basePoint.y),
                                                 data.radious,
                                                 L)).push();
    builder->execute();
}


void DXFimpl::addLayer(const DRW_Layer &data) {
    auto col = icol.intToColor(data.color);
    auto layer=std::make_shared<lc::Layer>(data.name, data.lWeight, col);
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();
}

lc::Document* DXFimpl::document() {
    return _document;
}

lc::StorageManager_SPtr DXFimpl::storageManager() {
    return _storageManager;
}
