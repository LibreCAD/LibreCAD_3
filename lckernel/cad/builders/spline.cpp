#include "spline.h"
#include <cad/primitive/spline.h>
#include <cstdint>
#include <stdexcept>

namespace {
std::size_t removalIndex(std::size_t size, int index) {
    if (index < 0) {
        const auto distance = static_cast<std::uint64_t>(-static_cast<std::int64_t>(index));
        if (distance > size) throw std::out_of_range("spline removal index");
        return size - static_cast<std::size_t>(distance);
    }
    const auto position = static_cast<std::size_t>(index);
    if (position >= size) throw std::out_of_range("spline removal index");
    return position;
}
}

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
    checkValues(true);

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

const std::vector<double>& lc::builder::SplineBuilder::weights() const {
    return _weights;
}

void lc::builder::SplineBuilder::setWeights(const std::vector<double>& weights) {
    _weights = weights;
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
    const auto position = removalIndex(_controlPoints.size(), index);
    if (!_weights.empty() && _weights.size() != _controlPoints.size())
        throw std::logic_error("spline weights do not match control points");
    _controlPoints.erase(_controlPoints.begin() + position);
    if (!_weights.empty()) _weights.erase(_weights.begin() + position);
}

void lc::builder::SplineBuilder::removeKnotPoint(int index) {
    _knotPoints.erase(_knotPoints.begin() + removalIndex(_knotPoints.size(), index));
}

void lc::builder::SplineBuilder::removeFitPoint(int index) {
    _fitPoints.erase(_fitPoints.begin() + removalIndex(_fitPoints.size(), index));
}

void lc::builder::SplineBuilder::copy(entity::Spline_CSPtr entity) {
    setDegree(entity->degree());
    setClosed(entity->closed());
    setFitTolerance(entity->fitTolerance());
    setStartTangent(lc::geo::Coordinate(entity->startTanX(), entity->startTanY(), entity->startTanZ()));
    setEndTangent(lc::geo::Coordinate(entity->endTanX(), entity->endTanY(), entity->endTanZ()));
    setNormalVector(lc::geo::Coordinate(entity->nX(), entity->nY(), entity->nZ()));
    setControlPoints(entity->controlPoints());
    setKnotPoints(entity->knotPoints());
    setFitPoints(entity->fitPoints());
    setWeights(entity->weights());
    setFlags(entity->flags());
    CADEntityBuilder::copy(entity);
}
