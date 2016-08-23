#pragma once

#include "cad/const.h"
#include "geoarea.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <cad/math/lcmath.h>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace lc {
    namespace geo {
    class BezierBase;
    using BB_CSPtr = std::shared_ptr<const BezierBase>;
    using BB_SPtr = std::shared_ptr<BezierBase>;

    class BezierBase  : public Base, virtual public Visitable {
            public:

                BezierBase() {}
                virtual ~BezierBase() {}

                /**
                 * @brief boundingBox of the bezier
                 * @return Area of the bounding box
                 */
                virtual const Area boundingBox() const = 0;

                /**
                 * @brief nearestPointOnPath of the bezier
                 * @param coord Coordinate to be compared with
                 * @return Coordinate of nearest point
                 */
                virtual Coordinate nearestPointOnPath(const Coordinate& coord) const = 0;

                /**
                 * @brief nearestPointOnEntity of the bezier
                 * @param coord Coordinate to be compared with
                 * @return Coordinate of nearest point
                 */
                virtual Coordinate nearestPointOnEntity(const Coordinate& coord) const = 0;

                /**
                 * @brief CasteljauAt Coordinate of bezier at time t
                 * @param points Vector of CP of bezier
                 * @param t Time t
                 * @return Coordinate at time t
                 */
                virtual Coordinate CasteljauAt(std::vector<Coordinate> points, double t) const = 0;

                /**
                 * @brief getCP Returns the control points of the bezier
                 * @return Vector of Coordinates.
                 */
                virtual const std::vector<Coordinate> getCP() const = 0;

                /**
                 * @brief DirectValueAt Coordinate of bezier at time t calculated numerically
                 * @param t Time t
                 * @return Coordinate at time t
                 */
                virtual Coordinate DirectValueAt(double t) const = 0;

                /**
                 * @brief Curve Creates a bezier curve for drawing with some precession value
                 * @param precession of the curve
                 * @return Vector of Coordinates to be joined.
                 */
                virtual const std::vector<Coordinate> Curve(double precession) = 0;

                /**
                 * @brief length of the Bezier
                 * @return double length
                 */
                virtual const double length() const = 0;

                /**
                 * @brief tangent of the bezeir
                 * @param t time T
                 * @return Coordinate of tangent
                 */
                virtual const Coordinate tangent(double t) const = 0;

                /**
                 * @brief normal of the bezier at time t
                 * @param t time t
                 * @return Coordinate of normal
                 */
                virtual const Coordinate normal(double t) const = 0;

                /**
                 * @brief splitHalf Splits the bezier into two halfs.
                 * @return Vector of two beziers.
                 */
                virtual std::vector<BB_CSPtr> splitHalf() const = 0;

                /**
                 * @brief splitAtT Splits bezier at specific time.
                 * @param t Time t
                 * @return Splitted bezier.
                 */
                virtual BB_CSPtr splitAtT(double t) const = 0;

                /**
                 * @brief offset offsets the bezier.
                 * @param offset offsect coordinate
                 * @return offsetted bezier
                 */
                virtual BB_CSPtr offset(const geo::Coordinate& offset) const = 0;

                /**
                 * @brief rotate the bezier at specific center and some angle
                 * @param center of rotation
                 * @param angle of rotation
                 * @return rotated bezier
                 */
                virtual BB_CSPtr rotate(const geo::Coordinate& center, double angle) const = 0;

                /**
                 * @brief scale the bezier at specific center by some factor
                 * @param center of scale
                 * @param factor of scale
                 * @return Scaled bezier
                 */
                virtual BB_CSPtr scale(const geo::Coordinate& center, const geo::Coordinate& factor) const = 0;

                /**
                 * @brief move the bezeir by some offset
                 * @param offset coordinate of move
                 * @return moved bezier
                 */
                virtual BB_CSPtr move(const geo::Coordinate& offset) const = 0;

                /**
                 * @brief mirror a bezier around a line
                 * @param axis1 first coordinate of line to be mirrored about
                 * @param axis2 second coordinate of line to be mirrored about
                 * @return mirrored bezier
                 */
                virtual BB_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const = 0;

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

                virtual std::vector<double> nearestPointTValue(const Coordinate &coord) const = 0;
                virtual const lc::geo::Coordinate returnCasesForNearestPoint(
                        double min_distance, const lc::geo::Coordinate &coord,
                        const Coordinate &ret) const = 0;
        };
    }
}
