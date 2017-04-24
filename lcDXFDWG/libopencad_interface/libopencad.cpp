#include "libopencad.h"

#include "../generic/helpers.h"
#include <cad/meta/icolor.h>
#include <cad/operations/layerops.h>
#include <cadcolors.h>


lc::FileLibs::LibOpenCad::LibOpenCad(lc::Document_SPtr document, lc::operation::Builder_SPtr builder) :
    _document(document),
    _builder(builder) {

}

void lc::FileLibs::LibOpenCad::open(const std::string& file) {
    auto f = OpenCADFile(file.c_str(), CADFile::OpenOptions::READ_ALL);

    if(f == nullptr) {
        std::cout << GetLastErrorCode() << std::endl;
        return;
    }

    auto layerCount = f->GetLayersCount();
    for(size_t i = 0; i < layerCount; i++) {
        auto layer = f->GetLayer(i);
        auto lcLayer = addLayer(layer);

        auto geometryCount = layer.getGeometryCount();
        for(size_t j = 0; j < geometryCount; j++) {
            auto geometry = layer.getGeometry(j);
            if(geometry != nullptr) {
                addGeometry(lcLayer, geometry);
            }
        }
    }

    free(f);
}

void lc::FileLibs::LibOpenCad::save(const std::string& file) {

}

lc::Layer_SPtr lc::FileLibs::LibOpenCad::addLayer(const CADLayer& data) {
    lc::iColor icolor;

    auto lw = lc::FileHelpers::intToLW(data.getLineWeight());
    auto color = icolor.intToColor(data.getColor())->color();

    auto layer = std::make_shared<Layer>(data.getName(), lw, color);
    std::make_shared<lc::operation::AddLayer>(_document, layer)->execute();

    return layer;
}

void lc::FileLibs::LibOpenCad::addGeometry(lc::Layer_SPtr layer, const CADGeometry* geometry) {
    switch(geometry->getType()) {
        case CADGeometry::ARC:
            addArc(layer, (CADArc*) geometry);
            break;
    }
}

void lc::FileLibs::LibOpenCad::addArc(lc::Layer_SPtr layer, const CADArc* arc) {
    auto position = arc->getPosition();
    auto lcCenter = lc::geo::Coordinate(position.getX(), position.getY(), position.getZ());

    auto lcArc = std::make_shared<lc::entity::Arc>(
            lcCenter, arc->getRadius(), arc->getStartingAngle(), arc->getEndingAngle(),
            true, layer, metaInfo(arc));

    _builder->append(lcArc);
}

lc::MetaInfo_SPtr lc::FileLibs::LibOpenCad::metaInfo(const CADGeometry* geometry) {
    auto metaInfo = MetaInfo::create();

    auto color = geometry->getColor();
    metaInfo->add(std::make_shared<lc::MetaColor>(lc::Color(color.R, color.G, color.B)));

    metaInfo->add(std::make_shared<lc::MetaLineWidthByValue>(geometry->getThickness()));

    return metaInfo;
}
