#include "trim.h"

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/math/geointersect.h"

using namespace lc;

Trim::Trim(geo::Coordinate trimPoint) : _trimPoint(trimPoint) {
}

void Trim::visitInteraction(LinePtr trimmedShape, LinePtr limitShape) {
    /*
    QList<CADEntityPtr> list;

    QList<geo::Coordinate> anyPoint = geo::GeoIntersect::intersectVectorVector(trimmedShape, limitShape, GeoIntersect::Any);
    QList<geo::Coordinate> interSectionPoint = geo::GeoIntersect::intersectVectorVector(trimmedShape, limitShape, GeoIntersect::MustIntersect);

    // If there are no intersection points we return a empty list
    if (anyPoint.count() == 0) {
        return list;
    }

    // When we have a true intersectionpint we remove from the trimmed entity
    if (interSectionPoint.count() != 0) {
        Coordinate iPoint = interSectionPoint.at(0);

        // Test if the intersectionpoint is the same as stop or start point
        if ((iPoint - trimmedShape.start()).magnitude() < 1.0e-4 || (iPoint - trimmedShape.end()).magnitude() < 1.0e-4) {
            return list;
        }

        // Create a new line based on the trim point and the end or start point
        if (Area(trimmedShape.start(), iPoint).inArea(removalPoint)) {
            list.append(Vector(trimmedShape.start(), iPoint));
        }

        if (Area(trimmedShape.end(), iPoint).inArea(removalPoint)) {
            list.append(Vector(iPoint, trimmedShape.end()));
        }
    } else { // no intersection point so we extend the trimmedShape
        Coordinate iPoint = anyPoint.at(0);
        double d1 = (trimmedShape.start() - iPoint).squared();
        double d2 = (trimmedShape.end() - iPoint).squared();

        // Create a new by extending the trimmed vector
        if (d1 < d2) {
            list.append(Vector(iPoint, trimmedShape.end()));
        } else {
            list.append(Vector(trimmedShape.start(), iPoint));
        }
    }
    */
    //   return list;
}
void Trim::visitInteraction(LinePtr, CirclePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(LinePtr, ArcPtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(LinePtr, EllipsePtr) {
    QList<CADEntityPtr> ret;
    return;
}



void Trim::visitInteraction(CirclePtr, LinePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(CirclePtr, CirclePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(CirclePtr, ArcPtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(CirclePtr, EllipsePtr) {
    QList<CADEntityPtr> ret;
    return;
}



void Trim::visitInteraction(ArcPtr, LinePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(ArcPtr, CirclePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(ArcPtr, ArcPtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(ArcPtr, EllipsePtr) {
    QList<CADEntityPtr> ret;
    return;
}



void Trim::visitInteraction(EllipsePtr, LinePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(EllipsePtr, CirclePtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(EllipsePtr, ArcPtr) {
    QList<CADEntityPtr> ret;
    return;
}
void Trim::visitInteraction(EllipsePtr, EllipsePtr) {
    QList<CADEntityPtr> ret;
    return;
}
