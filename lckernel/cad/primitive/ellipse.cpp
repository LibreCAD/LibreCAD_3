#include <memory>
#include <cad/interface/snapconstrain.h>
#include <cad/interface/snapable.h>
#include "ellipse.h"

using namespace lc;
using namespace entity;

Ellipse::Ellipse(const geo::Coordinate &center,
                 const geo::Coordinate &majorP,
                 double minorRadius,
                 double startAngle,
                 double endAngle,
                 bool reversed,
                 Layer_CSPtr layer,
                 MetaInfo_CSPtr metaInfo,
                 Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle, reversed) {
}

Ellipse::Ellipse(const Ellipse_CSPtr& other, bool sameID) :
        CADEntity(other, sameID),
        geo::Ellipse(other->center(),
                     other->majorP(),
                     other->minorRadius(),
                     other->startAngle(),
                     other->endAngle(),
                     other->isReversed()) {
}


CADEntity_CSPtr Ellipse::move(const geo::Coordinate &offset) const {
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

CADEntity_CSPtr Ellipse::copy(const geo::Coordinate &offset) const {
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

CADEntity_CSPtr Ellipse::rotate(const geo::Coordinate &rotation_center, double rotation_angle) const {
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

CADEntity_CSPtr Ellipse::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
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

CADEntity_CSPtr Ellipse::mirror(const geo::Coordinate &axis1, const geo::Coordinate &axis2) const {
    auto cen = this->center().mirror(axis1, axis2);
    auto maj = (cen + this->majorP()).mirror(axis1, axis2);
    auto majP = maj - cen;

    geo::Coordinate startP, endP;
    if (isArc()) {
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
    const std::vector<geo::Coordinate> points = findBoxPoints();
    double minX, minY, maxX, maxY;

    minX = points[0].x();
    maxX = points[0].x();
    minY = points[0].y();
    maxY = points[0].y();

    const auto checkPoint = [&](const geo::Coordinate& point) {
        if (point.x() < minX) {
            minX = point.x();
        }
        if (point.x() > maxX) {
            maxX = point.x();
        }
        if (point.y() < minY) {
            minY = point.y();
        }
        if (point.y() > maxY) {
            maxY = point.y();
        }
    };

    for (const auto& point : points) {
        checkPoint(point);
    }

    return geo::Area(geo::Coordinate(minX, minY),
                     geo::Coordinate(maxX, maxY));
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


std::vector<lc::geo::Coordinate> Ellipse::findBoxPoints() const {
    std::vector<lc::geo::Coordinate> resPoints;
    const double a = majorP().magnitude();
    const double b = this->minorRadius();
    const double angle = this->getAngle();
    geo::Coordinate c1, c2, c3, c4;
    bool simple = false; // (major axis = ox axis) or (major axis =  oy axis)

    if (sin(angle) == 0) {
        c1 = geo::Coordinate(this->center().x() + a, this->center().y());
        c2 = geo::Coordinate(this->center().x(), this->center().y() + b);
        c3 = geo::Coordinate(this->center().x() - a, this->center().y());
        c4 = geo::Coordinate(this->center().x(), this->center().y() - b);

        simple = true;
    }

    if (cos(angle) == 0) {
        c1 = geo::Coordinate(this->center().x() + b, this->center().y());
        c2 = geo::Coordinate(this->center().x(), this->center().y() + a);
        c3 = geo::Coordinate(this->center().x() - b, this->center().y());
        c4 = geo::Coordinate(this->center().x(), this->center().y() - a);

        simple = true;
    }

    if (!simple) {
        double tanAngle = std::tan(-this->getAngle());

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

        c1 = c1.rotate(angle) + this->center();
        c2 = c2.rotate(angle) + this->center();
        c3 = c3.rotate(angle) + this->center();
        c4 = c4.rotate(angle) + this->center();
    }

    if (this->isArc()) {
        resPoints.push_back(startPoint());
        resPoints.push_back(endPoint());
    }

    auto checkPoint = [&](geo::Coordinate cord) {
        if (isAngleBetween(getEllipseAngle(cord))) {
            resPoints.push_back(cord);
        }
    };

    checkPoint(c1);
    checkPoint(c2);
    checkPoint(c3);
    checkPoint(c4);

    return resPoints;
}


std::vector<EntityCoordinate> Ellipse::snapPoints(const geo::Coordinate &coord,
                                                  const SimpleSnapConstrain &constrain,
                                                  double minDistanceToSnap,
                                                  int maxNumberOfSnapPoints) const {

    std::vector<EntityCoordinate> resPoints;
    int ind = 0;

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        geo::Coordinate tmp;

        resPoints.emplace_back(this->center(), ind++);

        auto points = this->findBoxPoints();
        for (auto p : points) {
            resPoints.emplace_back(p, ind++);
        }

        geo::Coordinate minorP(this->majorP().angle());
        minorP = minorP.rotate(M_PI_2);
        minorP = minorP * this->minorRadius();

        if (sin(this->getAngle()) != 0 && cos(this->getAngle()) != 0) {
            //add Ellipse vertices
            tmp = center() + majorP();
            if (this->isAngleBetween(0)) {
                resPoints.emplace_back(tmp, ind++);
            }

            tmp = center() - majorP();
            if (this->isAngleBetween(M_PI)) {
                resPoints.emplace_back(tmp, ind++);
            }

            tmp = center() + minorP;
            if (this->isAngleBetween(M_PI_2)) {
                resPoints.emplace_back(tmp, ind++);
            }

            tmp = center() - minorP;
            if (this->isAngleBetween(-M_PI_2)) {
                resPoints.emplace_back(tmp, ind++);
            }
        }
    }

    // Any where on entity path
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH)) {
        geo::Coordinate nearestPoint = this->nearestPointOnPath(coord);
        resPoints.emplace_back(nearestPoint, ind++);
    }

    // Any where on entity
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY)) {
        geo::Coordinate nearestPoint = nearestPointOnEntity(coord);
        resPoints.emplace_back(nearestPoint, ind++);
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(resPoints, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return resPoints;
}

geo::Coordinate Ellipse::nearestPointOnPath(const geo::Coordinate &coord) const {
    return geo::Ellipse::nearestPointOnPath(coord);
}



