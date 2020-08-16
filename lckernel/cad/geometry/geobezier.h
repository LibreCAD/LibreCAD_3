#pragma once

#include "cad/const.h"
#include "geoarea.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <cad/math/lcmath.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include "geobezierbase.h"

namespace lc {
namespace geo {
class Bezier  : public BezierBase {
public:
    /**
     * Create a new Bezier
     */
    Bezier(Coordinate point_a, Coordinate point_b, Coordinate point_c);

    Bezier(const Bezier &bez);

    /**
     * @brief getCP Returns the control points of the bezier
     * @return Vector of Coordinates.
     */
    const std::vector<Coordinate> getCP() const override;

    /**
     * @brief boundingBox of the bezier
     * @return Area of the bounding box
     */
    const Area boundingBox() const override ;

    /**
     * @brief nearestPointOnPath of the bezier
     * @param coord Coordinate to be compared with
     * @return Coordinate of nearest point
     */
    Coordinate nearestPointOnPath(const Coordinate& coord) const override;

    /**
     * @brief nearestPointOnEntity of the bezier
     * @param coord Coordinate to be compared with
     * @return Coordinate of nearest point
     */
    Coordinate nearestPointOnEntity(const Coordinate& coord) const override;

    /**
     * @brief CasteljauAt Coordinate of bezier at time t
     * @param points Vector of CP of bezier
     * @param t Time t
     * @return Coordinate at time t
     */
    Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const override;

    /**
     * @brief DirectValueAt Coordinate of bezier at time t calculated numerically
     * @param t Time t
     * @return Coordinate at time t
     */
    Coordinate DirectValueAt(double t) const override;

    /**
     * @brief Curve Creates a bezier curve for drawing with some precession value
     * @param precession of the curve
     * @return Vector of Coordinates to be joined.
     */
    const std::vector<Coordinate> Curve(double precession) override;

    /**
     * @brief length of the Bezier
     * @return double length
     */
    const double length() const override;

    /**
     * @brief tangent of the bezeir
     * @param t time T
     * @return Coordinate of tangent
     */
    const Coordinate tangent(double t) const override;

    /**
     * @brief normal of the bezier at time t
     * @param t time t
     * @return Coordinate of normal
     */
    const Coordinate normal(double t) const override;

    /**
     * @brief splitHalf Splits the bezier into two halves.
     * @return Vector of two beziers.
     */
    std::vector<BB_CSPtr> splitHalf() const override;

    /**
     * @brief splitAtT Splits bezier at specific time.
     * @param t Time t
     * @return Split bezier.
     */
    BB_CSPtr splitAtT(double t) const override;

    /**
     * @brief offset offsets the bezier.
     * @param offset offsect coordinate
     * @return offsetted bezier
     */
    BB_CSPtr offset(const geo::Coordinate& offset) const override;

    /**
     * @brief rotate the bezier at specific center and some angle
     * @param center of rotation
     * @param angle of rotation
     * @return rotated bezier
     */
    BB_CSPtr rotate(const geo::Coordinate& center, double angle) const override;

    /**
     * @brief scale the bezier at specific center by some factor
     * @param center of scale
     * @param factor of scale
     * @return Scaled bezier
     */
    BB_CSPtr scale(const geo::Coordinate& center, const geo::Coordinate& factor) const override;
    /**
     * @brief move the bezeir by some offset
     * @param offset coordinate of move
     * @return moved bezier
     */
    BB_CSPtr move(const geo::Coordinate& offset) const override;
    /**
     * @brief mirror a bezier around a line
     * @param axis1 first coordinate of line to be mirrored about
     * @param axis2 second coordinate of line to be mirrored about
     * @return mirrored bezier
     */
    BB_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;

//                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }
private:

    std::vector<double> nearestPointTValue(const Coordinate &coord) const override;
    const lc::geo::Coordinate returnCasesForNearestPoint(
        double min_distance, const lc::geo::Coordinate &coord,
        const Coordinate &ret) const override;

    friend std::ostream& operator<<(std::ostream& os, const Bezier& bez) {
        os << "Bezier(A=" << bez._pointA << " radius=" << bez._pointB << " startAngle=" << bez._pointC << ")";
        return os;
    }

private:
    Coordinate _pointA;
    Coordinate _pointB;
    Coordinate _pointC;
};
}
}
