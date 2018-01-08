#include <memory>
#include "ellipse.h"

using namespace lc;
using namespace entity;

Ellipse::Ellipse(const geo::Coordinate& center,
                 const geo::Coordinate& majorP,
                 double minorRadius,
                 double startAngle,
                 double endAngle,
                 bool reversed,
                 const Layer_CSPtr layer,
                 const MetaInfo_CSPtr metaInfo,
                 const Block_CSPtr block) :
        CADEntity(layer, metaInfo, block),
        geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle, reversed) {
}

Ellipse::Ellipse(const Ellipse_CSPtr other, bool sameID) :
        CADEntity(other, sameID),
        geo::Ellipse(other->center(), other->majorP(), other->minorRadius(), other->startAngle(), other->endAngle(), other->isReversed()) {
}


CADEntity_CSPtr Ellipse::move(const geo::Coordinate& offset) const {
    auto newellipse = std::make_shared<Ellipse>(this->center() + offset,
                                                this->majorP(),
                                                this->minorRadius(),
                                                this->startAngle(), this->endAngle(),
                                                isReversed(),
                                                layer(),
                                                metaInfo(),
                                                block());
    newellipse->setID(this->id());
    return newellipse;
}

CADEntity_CSPtr Ellipse::copy(const geo::Coordinate& offset) const {
    auto newEllipse = std::make_shared<Ellipse>(this->center() + offset,
                                                this->majorP(),
                                                this->minorRadius(),
                                                this->startAngle(), this->endAngle(),
                                                isReversed(),
                                                layer(),
                                                metaInfo(),
                                                block());
    return newEllipse;
}

CADEntity_CSPtr Ellipse::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto rotated = this->georotate(rotation_center, rotation_angle);
    auto newEllipse = std::make_shared<Ellipse>(rotated.center(),
                                                rotated.majorP(),
                                                rotated.minorRadius(),
                                                rotated.startAngle(),
                                                rotated.endAngle(),
                                                isReversed(),
                                                layer(),
                                                metaInfo(),
                                                block()
    );
    newEllipse->setID(this->id());
    return newEllipse;
}

CADEntity_CSPtr Ellipse::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto scaled = this->geoscale(scale_center, scale_factor);
    auto newEllipse = std::make_shared<Ellipse>(scaled.center(),
                                                scaled.majorP(),
                                                scaled.minorRadius(),
                                                scaled.startAngle(),
                                                scaled.endAngle(),
                                                isReversed(),
                                                layer(),
                                                metaInfo(),
                                                block()
    );

    newEllipse->setID(this->id());
    return newEllipse;
}

CADEntity_CSPtr Ellipse::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto cen = this->center().mirror(axis1, axis2);
    auto maj = (cen + this->majorP()).mirror(axis1, axis2);
    auto majP = maj - cen;

    geo::Coordinate startP, endP;
    if(isArc()) {
        startP = startPoint().mirror(axis1, axis2);
        endP = endPoint().mirror(axis1, axis2);
    }

    auto newEllipse = std::make_shared<Ellipse>(cen, majP,
                                                minorRadius(),
                                                getEllipseAngle(startP),
                                                getEllipseAngle(endP),
                                                !isReversed(),
                                                layer(),
                                                metaInfo(),
                                                block()
    );
    newEllipse->setID(this->id());

    return newEllipse;
}

const geo::Area Ellipse::boundingBox() const {
    const double a = majorP().magnitude();
    const double b = this->minorRadius();
    const double angle = this->getAngle();
    geo::Coordinate c1, c2, c3, c4;
    double a1,a2,a3,a4;
    bool simple = false; // (major axis = ox axis) or (major axis =  oy axis)

    if (sin(angle) == 0) {
        c1 = geo::Coordinate(this->center().x() + a, this->center().y());
        a1 = 0;
        c2 = geo::Coordinate(this->center().x(), this->center().y() + b);
        a2 = 0.5*M_PI;
        c3 = geo::Coordinate(this->center().x() - a, this->center().y());
        a3 = 1.*M_PI;
        c4 = geo::Coordinate(this->center().x(), this->center().y() - b);
        a4 = 1.5*M_PI;

        simple = true;
    }

    if (cos(angle) == 0) {
        c1 = geo::Coordinate(this->center().x() + b, this->center().y());
        a1 = 0;
        c2 = geo::Coordinate(this->center().x(), this->center().y() + a);
        a2 = 0.5*M_PI;
        c3 = geo::Coordinate(this->center().x() - b, this->center().y());
        a3 = 1.0*M_PI;
        c4 = geo::Coordinate(this->center().x(), this->center().y() - a);
        a4 = 1.5*M_PI;        simple = true;
    }


    double minX, minY, maxX, maxY;


    geo::Coordinate sp = startPoint();
    minX = sp.x();
    maxX = sp.x();
    minY = sp.y();
    maxY = sp.y();

    auto checkPoint = [&](geo::Coordinate point, double angle) {
        if (this->isAngleBetween(angle)) {
            if (point.x() < minX)
                minX = point.x();
            if (point.x() > maxX)
                maxX = point.x();
            if (point.y() < minY)
                minY = point.y();
            if (point.y() > maxY)
                maxY = point.y();
        }
    };
    checkPoint(this->endPoint(), this->endAngle());

    if (!simple) {
        double tanAngle = std::tan(this->getAngle());

        auto getY = [&](double x) {
            return -b * b * x / (a * a * tanAngle);
        };

        double x = a * a * tanAngle / (sqrt(a * a * tanAngle * tanAngle + b * b));
        c1 = geo::Coordinate(x, getY(x));
        c2 = geo::Coordinate(-x, getY(-x));

        tanAngle = -1 / tanAngle;
        x = a * a * tanAngle / (sqrt(a * a * tanAngle * tanAngle + b * b));
        c3 = geo::Coordinate(x, getY(x));
        c4 = geo::Coordinate(-x, getY(-x));
        a1 = c1.angle()-angle;
        a2 = c2.angle()-angle;
        a3 = c3.angle()-angle;
        a4 = c4.angle()-angle;

        c1 = c1.rotate(-angle) + this->center();
        c2 = c2.rotate(-angle) + this->center();
        c3 = c3.rotate(-angle) + this->center();
        c4 = c4.rotate(-angle) + this->center();
    }

    checkPoint(c1,a1);
    checkPoint(c2,a2);
    checkPoint(c3,a3);
    checkPoint(c4,a4);

    return geo::Area(geo::Coordinate(minX,minY),
                     geo::Coordinate(maxX,maxY));
}

CADEntity_CSPtr Ellipse::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo, Block_CSPtr block) const {
    auto newEntity = std::make_shared<Ellipse>(
            this->center(),
            this->majorP(),
            this->minorRadius(),
            this->startAngle(),
            this->endAngle(),
            this->isReversed(),
            layer,
            metaInfo,
            block
    );

    newEntity->setID(this->id());

    return newEntity;
}


