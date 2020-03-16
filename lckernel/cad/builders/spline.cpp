#include "spline.h"
#include <cad/primitive/spline.h>

lc::builder::SplineBuilder::SplineBuilder() {
    _degree = 2;
    _closed = false;
    _fitTolerance = 0;
    _flags = (lc::entity::Spline::splineflag) 0;
}

short lc::builder::SplineBuilder::degree() const {
    return _degree;
}

void lc::builder::SplineBuilder::setDegree(short degree) {
    _degree = degree;
}

bool lc::builder::SplineBuilder::closed() const {
    return _closed;
}

void lc::builder::SplineBuilder::setClosed(bool closed) {
    _closed = closed;
}

double lc::builder::SplineBuilder::fitTolerance() const {
    return _fitTolerance;
}

void lc::builder::SplineBuilder::setFitTolerance(double fitTolerance) {
    _fitTolerance = fitTolerance;
}

const lc::geo::Coordinate& lc::builder::SplineBuilder::startTangent() const {
    return _startTangent;
}

void lc::builder::SplineBuilder::setStartTangent(const lc::geo::Coordinate& startTangent) {
    _startTangent = startTangent;
}

const lc::geo::Coordinate& lc::builder::SplineBuilder::endTangent() const {
    return _endTangent;
}

void lc::builder::SplineBuilder::setEndTangent(const lc::geo::Coordinate& endTangent) {
    _endTangent = endTangent;
}

const lc::geo::Coordinate& lc::builder::SplineBuilder::normalVector() const {
    return _normalVector;
}

void lc::builder::SplineBuilder::setNormalVector(const lc::geo::Coordinate& normalVector) {
    _normalVector = normalVector;
}

void lc::builder::SplineBuilder::addControlPoint(const lc::geo::Coordinate& controlPoint) {
    _controlPoints.push_back(controlPoint);
}

void lc::builder::SplineBuilder::addKnotPoint(double knotPoint) {
    _knotPoints.push_back(knotPoint);
}

void lc::builder::SplineBuilder::addFitPoint(lc::geo::Coordinate fitPoint) {
    _fitPoints.push_back(std::move(fitPoint));
}

lc::entity::Spline_CSPtr lc::builder::SplineBuilder::build() const {
    checkEntityConstraints();

    return lc::entity::Spline_CSPtr(new lc::entity::Spline(*this));
}

const std::vector<lc::geo::Coordinate>& lc::builder::SplineBuilder::controlPoints() const {
    return _controlPoints;
}

void lc::builder::SplineBuilder::setControlPoints(const std::vector<lc::geo::Coordinate>& controlPoints) {
    _controlPoints = controlPoints;
}

const std::vector<double>& lc::builder::SplineBuilder::knotPoints() const {
    return _knotPoints;
}

void lc::builder::SplineBuilder::setKnotPoints(const std::vector<double>& knotPoints) {
    _knotPoints = knotPoints;
}

const std::vector<lc::geo::Coordinate>& lc::builder::SplineBuilder::fitPoints() const {
    return _fitPoints;
}

void lc::builder::SplineBuilder::setFitPoints(const std::vector<lc::geo::Coordinate>& fitPoints) {
    _fitPoints = fitPoints;
}

const lc::geo::Spline::splineflag lc::builder::SplineBuilder::flags() const {
    return _flags;
}

void lc::builder::SplineBuilder::setFlags(lc::geo::Spline::splineflag flags) {
    _flags = flags;
}

void lc::builder::SplineBuilder::removeControlPoint(int index) {
    if(index < 0) {
        _controlPoints.erase(_controlPoints.end() + index);
    }
    else {
        _controlPoints.erase(_controlPoints.begin() + index);
    }
}

void lc::builder::SplineBuilder::removeKnotPoint(int index) {
    if(index < 0) {
        _knotPoints.erase(_knotPoints.end() + index);
    }
    else {
        _knotPoints.erase(_knotPoints.begin() + index);
    }
}

void lc::builder::SplineBuilder::removeFitPoint(int index) {
    if(index < 0) {
        _fitPoints.erase(_fitPoints.end() + index);
    }
    else {
        _fitPoints.erase(_fitPoints.begin() + index);
    }
}
