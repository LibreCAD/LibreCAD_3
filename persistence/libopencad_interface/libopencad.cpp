#ifdef LIBOPENCAD_ENABLED
#include "libopencad.h"

#include "../generic/helpers.h"
#include <cad/meta/icolor.h>
#include <cad/operations/layerops.h>

lc::persistence::LibOpenCad::LibOpenCad(lc::storage::Document_SPtr document, lc::operation::Builder_SPtr builder) :
    _document(std::move(document)),
    _builder(std::move(builder)),
    _entityBuilder(std::make_shared<lc::operation::EntityBuilder>(document)) {
}

void lc::persistence::LibOpenCad::open(const std::string& file) {
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

void lc::persistence::LibOpenCad::save(const std::string& file) {

}

lc::meta::Layer_SPtr lc::persistence::LibOpenCad::addLayer(const CADLayer& layer) {
    lc::iColor icolor;

    auto lw = lc::persistence::FileHelpers::intToLW(layer.getLineWeight());
    auto color = icolor.intToColor(layer.getColor())->color();

    auto lcLayer = std::make_shared<meta::Layer>(layer.getName(), lw, color);

    if(layer.getName() == "0") {
        _builder->append(std::make_shared<lc::operation::AddLayer>(_document, lcLayer));
    }
    else {
        _builder->append(std::make_shared<lc::operation::ReplaceLayer>(_document, _document->layerByName("0"), lcLayer));
    }

    return lcLayer;
}

void lc::persistence::LibOpenCad::addGeometry(const lc::meta::Layer_SPtr& layer, const CADGeometry* geometry) {
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

void lc::persistence::LibOpenCad::addArc(lc::meta::Layer_SPtr layer, const CADArc* arc) {
    auto position = arc->getPosition();

    auto lcArc = std::make_shared<lc::entity::Arc>(
                     toLcPostiton(position), arc->getRadius(), arc->getStartingAngle(), arc->getEndingAngle(),
                     true, layer, metaInfo(arc));

    _entityBuilder->appendEntity(lcArc);
}

void lc::persistence::LibOpenCad::addLine(lc::meta::Layer_SPtr layer, const CADLine* line) {
    auto lcLine = std::make_shared<lc::entity::Line>(
                      toLcPostiton(line->getStart().getPosition()),
                      toLcPostiton(line->getEnd().getPosition()),
                      layer,
                      metaInfo(line)
                  );

    _entityBuilder->appendEntity(lcLine);
}

void lc::persistence::LibOpenCad::addCircle(lc::meta::Layer_SPtr layer, const CADCircle* circle) {
    auto lcCircle = std::make_shared<lc::entity::Circle>(
                        toLcPostiton(circle->getPosition()),
                        circle->getRadius(),
                        layer,
                        metaInfo(circle)
                    );

    _entityBuilder->appendEntity(lcCircle);
}

void lc::persistence::LibOpenCad::addEllipse(lc::meta::Layer_SPtr layer, const CADEllipse* ellipse) {
    auto lcEllipse = std::make_shared<lc::entity::Ellipse>(
                         toLcPostiton(ellipse->getPosition()),
                         toLcPostiton(ellipse->getSMAxis()),
                         ellipse->getRadius(),
                         ellipse->getStartingAngle(),
                         ellipse->getEndingAngle(),
                         false,
                         layer,
                         metaInfo(ellipse)
                     );

    _entityBuilder->appendEntity(lcEllipse);
}

void lc::persistence::LibOpenCad::addLWPolyline(lc::meta::Layer_SPtr layer, const CADLWPolyline* lwPolyline) {
    auto lcVertices = std::vector<lc::entity::LWVertex2D>();

    size_t count = lwPolyline->getVertexCount();
    for(size_t i = 0; i < count; i++) {
        const auto& vertex = lwPolyline->getVertex(i);
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

    _entityBuilder->appendEntity(lcLWPolyline);
}

lc::meta::MetaInfo_SPtr lc::persistence::LibOpenCad::metaInfo(const CADGeometry* geometry) {
    auto metaInfo = meta::MetaInfo::create();

    auto color = geometry->getColor();
    metaInfo->add(std::make_shared<lc::meta::MetaColorByValue>(lc::Color(color.R, color.G, color.B)));

    metaInfo->add(std::make_shared<lc::meta::MetaLineWidthByValue>(geometry->getThickness()));

    return metaInfo;
}

lc::geo::Coordinate lc::persistence::LibOpenCad::toLcPostiton(const CADVector& position) {
    return {position.getX(), position.getY(), position.getZ()};
}
#endif
