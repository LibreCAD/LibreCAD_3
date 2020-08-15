#pragma once

#include "cadentity.h"
#include <cad/geometry/geospline.h>

namespace lc {
namespace builder {
class SplineBuilder : public CADEntityBuilder {
public:
    SplineBuilder();

    short degree() const;
    void setDegree(short degree);

    bool closed() const;
    void setClosed(bool closed);

    double fitTolerance() const;
    void setFitTolerance(double fitTolerance);

    const geo::Coordinate& startTangent() const;
    void setStartTangent(const geo::Coordinate& startTangent);

    const geo::Coordinate& endTangent() const;
    void setEndTangent(const geo::Coordinate& endTangent);

    const geo::Coordinate& normalVector() const;
    void setNormalVector(const geo::Coordinate& normalVector);

    const std::vector<geo::Coordinate>& controlPoints() const;
    void setControlPoints(const std::vector<geo::Coordinate>& controlPoints);
    void addControlPoint(const lc::geo::Coordinate& controlPoint);
    /**
     * @brief Remove control point
     * @param index Index of the element to remove, or negative to remove from the end
     */
    void removeControlPoint(int index);

    const std::vector<double>& knotPoints() const;
    void setKnotPoints(const std::vector<double>& knotPoints);
    void addKnotPoint(double knotPoint);
    /**
     * @brief Remove knot point
     * @param index Index of the element to remove, or negative to remove from the end
     */
    void removeKnotPoint(int index);

    const std::vector<geo::Coordinate>& fitPoints() const;
    void setFitPoints(const std::vector<geo::Coordinate>& fitPoints);
    void addFitPoint(geo::Coordinate fitPoint);

    /**
     * @brief Remove fit point
     * @param index Index of the element to remove, or negative to remove from the end
     */
    void removeFitPoint(int index);

    const geo::Spline::splineflag flags() const;
    void setFlags(geo::Spline::splineflag flags);

    lc::entity::Spline_CSPtr build() const;

    void copy(entity::Spline_CSPtr entity);

private:
    std::vector<geo::Coordinate> _controlPoints;
    std::vector<double> _knotPoints;
    std::vector<geo::Coordinate> _fitPoints;

    short _degree;
    bool _closed;
    double _fitTolerance;

    geo::Coordinate _startTangent;
    geo::Coordinate _endTangent;
    geo::Coordinate _normalVector;

    lc::geo::Spline::splineflag _flags;
};
}
}
