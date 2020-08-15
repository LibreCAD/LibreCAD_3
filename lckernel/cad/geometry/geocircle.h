#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <vector>
#include <cad/interface/tangentable.h>
#include "cad/math/equation.h"
namespace lc {
namespace geo {
// TODO: Should Circle be a arc with a start angle of 0 and a stop angle of 2PI ?
class Circle : public Base, virtual public Visitable, public entity::Tangentable {
public:
    Circle(Coordinate center, double radius);

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
    Coordinate nearestPointOnEntity(const Coordinate& coord) const;

    /**
     * @brief Returns equation of the circle
     * @return Equation
     */
    const maths::Equation equation() const {
        return maths::Equation(1., 0.,1., 0., 0., -_radius* _radius).move(_center);
    }

    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }

    std::vector<Coordinate> lineTangentPointsOnEntity(const lc::geo::Coordinate& extPoint) override;

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
