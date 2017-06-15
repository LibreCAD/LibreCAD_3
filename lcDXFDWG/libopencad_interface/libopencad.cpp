#include "libopencad.h"

#include "../generic/helpers.h"
#include <cad/meta/icolor.h>
#include <cad/operations/layerops.h>

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

    if(data.getName() == "0") {
        std::make_shared<lc::operation::AddLayer>(_document, layer)->execute();
    }
    else {
        std::make_shared<lc::operation::ReplaceLayer>(_document, _document->layerByName("0"), layer)->execute();
    }

    return layer;
}

void lc::FileLibs::LibOpenCad::addGeometry(lc::Layer_SPtr layer, const CADGeometry* geometry) {
    switch(geometry->getType()) {
        case CADGeometry::ARC:
            addArc(layer, (CADArc*) geometry);
            break;

        case CADGeometry::LINE:
            addLine(layer, (CADLine*) geometry);
            break;

        case CADGeometry::CIRCLE:
            addCircle(layer, (CADCircle*) geometry);
            break;

        case CADGeometry::ELLIPSE:
            addEllipse(layer, (CADEllipse*) geometry);
            break;

        case CADGeometry::LWPOLYLINE:
            addLWPolyline(layer, (CADLWPolyline*) geometry);
            break;

        default: //TODO: implement missing entities
        case CADGeometry::UNDEFINED:
            break;
    }
}

void lc::FileLibs::LibOpenCad::addArc(lc::Layer_SPtr layer, const CADArc* arc) {
    auto position = arc->getPosition();

    auto lcArc = std::make_shared<lc::entity::Arc>(
            toLcPostiton(position), arc->getRadius(), arc->getStartingAngle(), arc->getEndingAngle(),
            true, layer, metaInfo(arc));

    _builder->append(lcArc);
}

void lc::FileLibs::LibOpenCad::addLine(lc::Layer_SPtr layer, const CADLine* line) {
    auto lcLine = std::make_shared<lc::entity::Line>(
            toLcPostiton(line->getStart().getPosition()),
            toLcPostiton(line->getEnd().getPosition()),
            layer,
            metaInfo(line)
    );

    _builder->append(lcLine);
}

void lc::FileLibs::LibOpenCad::addCircle(lc::Layer_SPtr layer, const CADCircle* circle) {
    auto lcCircle = std::make_shared<lc::entity::Circle>(
            toLcPostiton(circle->getPosition()),
            circle->getRadius(),
            layer,
            metaInfo(circle)
    );

    _builder->append(lcCircle);
}

void lc::FileLibs::LibOpenCad::addEllipse(lc::Layer_SPtr layer, const CADEllipse* ellipse) {
    auto lcEllipse = std::make_shared<lc::entity::Ellipse>(
            toLcPostiton(ellipse->getPosition()),
            toLcPostiton(ellipse->getSMAxis()),
            ellipse->getRadius(),
            ellipse->getStartingAngle(),
            ellipse->getEndingAngle(),
            layer,
            metaInfo(ellipse)
    );

    _builder->append(lcEllipse);
}

void lc::FileLibs::LibOpenCad::addLWPolyline(lc::Layer_SPtr layer, const CADLWPolyline* lwPolyline) {
    auto lcVertices = std::vector<lc::entity::LWVertex2D>();

    size_t count = lwPolyline->getVertexCount();
    for(size_t i = 0; i < count; i++) {
        auto vertex = lwPolyline->getVertex(i);
        lcVertices.emplace_back(
                lc::geo::Coordinate(vertex.getX(), vertex.getY(), vertex.getZ())
        );
    }

    auto lcLWPolyline = std::make_shared<lc::entity::LWPolyline>(
            lcVertices,
            lwPolyline->getConstWidth(),
            lwPolyline->getElevation(),
            lwPolyline->getThickness(),
            lwPolyline->isClosed(),
            toLcPostiton(lwPolyline->getVectExtrusion()),
            layer,
            metaInfo(lwPolyline)
    );

    _builder->append(lcLWPolyline);
}

lc::MetaInfo_SPtr lc::FileLibs::LibOpenCad::metaInfo(const CADGeometry* geometry) {
    auto metaInfo = MetaInfo::create();

    auto color = geometry->getColor();
    metaInfo->add(std::make_shared<lc::MetaColor>(lc::Color(color.R, color.G, color.B)));

    metaInfo->add(std::make_shared<lc::MetaLineWidthByValue>(geometry->getThickness()));

    return metaInfo;
}

lc::geo::Coordinate lc::FileLibs::LibOpenCad::toLcPostiton(const CADVector& position) {
    return lc::geo::Coordinate(position.getX(), position.getY(), position.getZ());
}
