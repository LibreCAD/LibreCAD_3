#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <vector>
#include "cad/math/quadratic_math.h"

namespace lc {
    namespace geo {
        // TODO: Should Circle be a arc with a start angle of 0 and a stop angle of 2PI ?
        class Circle : public Base, virtual public Visitable {
            public:
                Circle(const Coordinate& center, double radius);

                /*!
                 * \brief Returns the Center of circle.
                 * \return lc::geo::Coordinate Center.
                 */
                const Coordinate center() const;

                /*!
                 * \brief returns the radius of the circle.
                 * \return Double radius.
                 */
                double radius() const;

                /*!
                 * \brief returns the nearest Point On Path
                 * \param lc::geo::Coordinate coord
                 * \return lc::geo::Coordinate
                 */
                Coordinate nearestPointOnPath(const Coordinate& coord) const;

                /*!
                 * \brief checks if Coordinate is on path
                 * \param lc::geo::Coordinate coord
                 * \return bool
                 */
                bool isCoordinateOnPath(const Coordinate& coord) const;

                 /**
                 * @brief quadratic, Returns quadratic for the entity
                 * @return Quadratic quadratic equation
                 */
                Quadratic quadratic() const {
                    std::vector<double> ce(6, 0.);
                    ce[0] = 1.;
                    ce[2] = 1.;
                    ce[5] = -this->radius() * this->radius();
                    Quadratic ret(ce);
                    ret.move(this->center());
                    return ret;
                }

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

        private:
                friend std::ostream& operator<<(std::ostream& os, const Circle& c) {
                    os << "Circle(center=" << c._center << " radius" << c._radius << ")";
                    return os;
                }
            private:
                const Coordinate _center; /*!< center of cirlce */
                const double _radius; /*!< Radius of circle. */
        };
    }
}
