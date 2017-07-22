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
        geo::Ellipse(other->center(), other->majorP(), other->minorRadius(), other->startAngle(), other->endAngle()) {
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

    // http://fridrich.blogspot.nl/2011/06/bounding-box-of-svg-elliptical-arc.html
    //geo::Coordinate p = geo::Coordinate(this->center().x(), this->center().y() + this->minorRadius()).rotate(this->majorP() - this->center());

    //    double r_min = this->minorRadius();
    //    double r_max = p.distanceTo(this->majorP());
    //    double rot = (this->majorP() - this->center()).angle();

    //    double t_nil = atan(-r_min * tan(rot) / r_max);
    //    double t_inf = atan(r_min * (cos(rot) / sin(rot)) / r_max);
    //    double rect_width = 2 * (r_max * cos(t_nil) * cos(rot) - r_min * sin(t_nil) * sin(rot));
    //    double rect_height = 2 * (r_min * sin(t_inf) * cos(rot) + r_max * cos(t_inf) * sin(rot));

    //   return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
    /* TODO implement
     * fix compiler warning
     */
    return geo::Area();
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


