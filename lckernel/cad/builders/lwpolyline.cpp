#include "lwpolyline.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/interface/metatype.h"
#include <math.h>

using namespace lc::builder;

LWPolylineBuilder::LWPolylineBuilder()
	:
	_currentVertex_Bulge(1),
	_currentVertex_StartWidth(0),
	_currentVertex_EndWidth(0),
	_currentVertex_Location(),
    _width(1),
    _elevation(1),
    _thickness(1),
    _closed(false),
    _extrusionDirection(lc::geo::Coordinate(0, 0))
{}

void LWPolylineBuilder::addLineVertex(const lc::geo::Coordinate& vert)
{
	_vertices.push_back(lc::builder::LWBuilderVertex(vert, 0));
	_currentVertex_Location = vert;
}

void LWPolylineBuilder::addArcVertex(const lc::geo::Coordinate& vert)
{
	_vertices.push_back(lc::builder::LWBuilderVertex(vert, _currentVertex_Bulge));
	_currentVertex_Location = vert;
}

void LWPolylineBuilder::modifyLastVertex(const geo::Coordinate& data)
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_currentVertex_Bulge = tan(data.magnitude() / 4);
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, vert.endWidth, _currentVertex_Bulge);
}

void LWPolylineBuilder::modifyLastVertexArc()
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, vert.endWidth, _currentVertex_Bulge);
}

void LWPolylineBuilder::modifyLastVertexLine()
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, vert.endWidth, 0);
}

const std::vector<lc::builder::LWBuilderVertex>& LWPolylineBuilder::getVertices()
{
	return _vertices;
}

lc::entity::LWPolyline_CSPtr LWPolylineBuilder::build()
{
    checkValues(true);

	return lc::entity::LWPolyline_CSPtr(new lc::entity::LWPolyline(*this));
}

void LWPolylineBuilder::removeVertex(int index)
{
	if (index < 0) {
		_vertices.erase(_vertices.end() + index);
	}
	else {
		_vertices.erase(_vertices.begin() + index);
	}
}

void LWPolylineBuilder::copy(entity::LWPolyline_CSPtr entity) {
    const std::vector<lc::entity::LWVertex2D>& verticesList = entity->vertex();

    for (const lc::entity::LWVertex2D& vert : verticesList) {
        _vertices.push_back(lc::builder::LWBuilderVertex(vert.location(), vert.startWidth(), vert.endWidth(), vert.bulge()));
    }

    _width = entity->width();
    _elevation = entity->elevation();
    _thickness = entity->tickness();
    _closed = entity->closed();
    _extrusionDirection = entity->extrusionDirection();

    lc::builder::CADEntityBuilder::copy(entity);
}

double LWPolylineBuilder::width() const {
    return _width;
}

double LWPolylineBuilder::elevation() const {
    return _elevation;
}

double LWPolylineBuilder::thickness() const {
    return _thickness;
}

bool LWPolylineBuilder::closed() const {
    return _closed;
}

lc::geo::Coordinate LWPolylineBuilder::extrusionDirection() const {
    return _extrusionDirection;
}

void LWPolylineBuilder::setVertices(const std::vector<lc::builder::LWBuilderVertex>& builderVertices) {
    _vertices.clear();

    for (const LWBuilderVertex& vert : builderVertices) {
        _vertices.push_back(vert);
    }
}
