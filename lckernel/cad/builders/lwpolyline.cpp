#include "lwpolyline.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/interface/metatype.h"
#include <math.h>

lc::builder::LWPolylineBuilder::LWPolylineBuilder()
	:
	_currentVertex_Bulge(1),
	_currentVertex_StartWidth(0),
	_currentVertex_EndWidth(0),
	_currentVertex_Location()
{}

void lc::builder::LWPolylineBuilder::addLineVertex(const lc::geo::Coordinate& vert)
{
	_vertices.push_back(lc::builder::LWBuilderVertex(vert, 0));
	_currentVertex_Location = vert;
}

void lc::builder::LWPolylineBuilder::addArcVertex(const lc::geo::Coordinate& vert)
{
	_vertices.push_back(lc::builder::LWBuilderVertex(vert, _currentVertex_Bulge));
	_currentVertex_Location = vert;
}

void lc::builder::LWPolylineBuilder::modifyLastVertex(const geo::Coordinate& data)
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_currentVertex_Bulge = tan(data.magnitude() / 4);
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, vert.endWidth, _currentVertex_Bulge);
}

void lc::builder::LWPolylineBuilder::modifyLastVertexArc()
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, vert.endWidth, _currentVertex_Bulge);
}

void lc::builder::LWPolylineBuilder::modifyLastVertexLine()
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, vert.endWidth, 0);
}

const std::vector<lc::builder::LWBuilderVertex>& lc::builder::LWPolylineBuilder::getVertices()
{
	return _vertices;
}

lc::entity::LWPolyline_CSPtr lc::builder::LWPolylineBuilder::build()
{
    checkEntityConstraints();

	return lc::entity::LWPolyline_CSPtr(new lc::entity::LWPolyline(*this));
}

void lc::builder::LWPolylineBuilder::removeVertex(int index)
{
	if (index < 0) {
		_vertices.erase(_vertices.end() + index);
	}
	else {
		_vertices.erase(_vertices.begin() + index);
	}
}
