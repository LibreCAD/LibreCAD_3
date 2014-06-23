#include "cad/primitive/dimlinear.h"

using namespace lc;

DimLinear::DimLinear(const Dimension& dimension,
                     const geo::Coordinate& extension_point1,
                     const geo::Coordinate& extension_point2,
                     const double oblique,
                     const Layer_CSPtr layer): CADEntity(layer), geo::DimLinear(dimension, extension_point1, extension_point2, oblique) {

}

DimLinear::DimLinear(const Dimension& dimension,
                     const geo::Coordinate& extension_point1,
                     const geo::Coordinate& extension_point2,
                     const double oblique,
                     const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes): CADEntity(layer), geo::DimLinear(dimension, extension_point1, extension_point2, oblique) {

}


CADEntity_CSPtr DimLinear::move(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimLinear = std::make_shared<DimLinear>(newDimension, this->extension_point1() + offset,
                                                    this->extension_point2() + offset, this->oblique(), layer());
    newDimLinear->setID(this->id());
    return newDimLinear;
}

CADEntity_CSPtr DimLinear::copy(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimLinear = std::make_shared<DimLinear>(newDimension, this->extension_point1() + offset,
                                                    this->extension_point2() + offset, this->oblique(), layer());
    return newDimLinear;
}

CADEntity_CSPtr DimLinear::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = Dimension(this->definition_point().rotate(rotation_center, rotation_angle),
                                  this->middle_of_text().rotate(rotation_center, rotation_angle),
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), Math::correctAngle(this->angle() + rotation_angle));
    auto newDimLinear = std::make_shared<DimLinear>(newDimension, this->extension_point1().rotate(rotation_center, rotation_angle),
                                                    this->extension_point2().rotate(rotation_center, rotation_angle),
                                                    Math::correctAngle(this->oblique() + rotation_angle), layer());
    newDimLinear->setID(this->id());
    return newDimLinear;
}

CADEntity_CSPtr DimLinear::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimension = Dimension(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
                                  this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimLinear = std::make_shared<DimLinear>(newDimension, this->extension_point1().scale(scale_center, scale_factor),
                                                    this->extension_point2().scale(scale_center, scale_factor),
                                                    this->oblique(), layer());
    newDimLinear->setID(this->id());
    return newDimLinear;
}

const geo::Area DimLinear::boundingBox() const {
    // TODO create bounding box for DimLinear
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}

Quadratic_CSPtr DimLinear::quadratic_equation() const {

}
