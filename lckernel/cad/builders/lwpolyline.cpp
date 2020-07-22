#include "lwpolyline.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/interface/metatype.h"
#include "cad/math/lcmath.h"
#include <math.h>

lc::builder::LWPolylineBuilder::LWPolylineBuilder()
	:
	_currentVertex_Bulge(1),
	_currentVertex_StartWidth(0),
	_currentVertex_EndWidth(0)
{}

void lc::builder::LWPolylineBuilder::addLineVertex(const lc::geo::Coordinate& vert)
{
	int n = _vertices.size();
	if(n<2){
		_vertices.push_back(lc::builder::LWBuilderVertex(vert, _currentVertex_StartWidth, _currentVertex_EndWidth, 0, 0));
		return;
	}
	// There is chance of having arc before it
	auto vert1 = _vertices[n - 1];
	auto vert2 = _vertices[n - 2];
	if(vert2.bulge==0){
		_vertices.push_back(lc::builder::LWBuilderVertex(vert, _currentVertex_StartWidth, _currentVertex_EndWidth, 0, 0));
		return;
	}else{
		// There is arc so create continuation to it
		auto delta = vert-vert1.location;
		auto angle =  vert2.location.angleTo(vert1.location) + vert2.hintAngle;//End angle
		auto a = geo::Coordinate(cos(angle),sin(angle));
		auto magnitude = delta.dot(a);
		if(magnitude<0)magnitude=0;
		_vertices.push_back(lc::builder::LWBuilderVertex(vert1.location + a*magnitude, _currentVertex_StartWidth, _currentVertex_EndWidth, 0, 0));
		// This may be changed to requesting length for extruction
	}
}

void lc::builder::LWPolylineBuilder::addArcVertex(const lc::geo::Coordinate& vert)
{
	int n = _vertices.size();
	if(n<2){
		_vertices.push_back(lc::builder::LWBuilderVertex(vert, _currentVertex_StartWidth, _currentVertex_EndWidth, 1, M_PI/2));
		// If no information than it's semi circle
		// So bulge is hand coded 1 for now
		return;
	}
	auto vert1 = _vertices[n - 1];
	auto vert2 = _vertices[n - 2];
	double bulge,angle;
	angle=-vert2.location.angleTo(vert1.location)+vert1.location.angleTo(vert) - vert2.hintAngle;
	bulge=(1-cos(angle))/sin(angle);
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert1.location, vert1.startWidth, vert1.endWidth, bulge, angle);
	_vertices.push_back(lc::builder::LWBuilderVertex(vert, _currentVertex_StartWidth, _currentVertex_EndWidth, _currentVertex_Bulge, 0));
}

void lc::builder::LWPolylineBuilder::modifyLastVertex(const geo::Coordinate& data)
{
	// This is called after addVertex to apply width...
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, _currentVertex_StartWidth, _currentVertex_EndWidth, vert.bulge, vert.hintAngle);
}

void lc::builder::LWPolylineBuilder::modifyLastVertexArc()
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	double bulge=vert.bulge, angle=vert.hintAngle;
	if(bulge==0){
		bulge=1;// Use Semi-circle
		angle=M_PI/2;// close at 90 deg
		}
	// We can use _currentVertexBulge here but we have to determine angle here too by calculation
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, _currentVertex_StartWidth, _currentVertex_EndWidth, bulge, angle);
}

void lc::builder::LWPolylineBuilder::modifyLastVertexLine()
{
	int n = _vertices.size();
	lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
	_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, _currentVertex_StartWidth, _currentVertex_EndWidth, 0, 0);
}

void lc::builder::LWPolylineBuilder::setWidth(double width){
	_currentVertex_StartWidth = width;
	_currentVertex_EndWidth = width;
	// Let's make it smooth
	int n = _vertices.size();
	if (n>=1){//Change endWidth of previous entity to make smooth transition
		lc::builder::LWBuilderVertex& vert = _vertices[n - 1];
		_vertices[n - 1] = lc::builder::LWBuilderVertex(vert.location, vert.startWidth, width, vert.bulge, vert.hintAngle);
	}
}

const std::vector<lc::builder::LWBuilderVertex>& lc::builder::LWPolylineBuilder::getVertices()
{
	return _vertices;
}

lc::entity::LWPolyline_CSPtr lc::builder::LWPolylineBuilder::build()
{
    checkValues(true);

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
