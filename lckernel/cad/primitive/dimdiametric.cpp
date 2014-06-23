#include "cad/primitive/dimdiametric.h"

using namespace lc;

DimDiametric::DimDiametric(const Dimension& dimension,
                           const double leader,
                           const Layer_CSPtr layer): CADEntity(layer), geo::DimDiametric(dimension, leader) {

}

DimDiametric::DimDiametric(const Dimension& dimension,
                           const double leader,
                           const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes): CADEntity(layer), geo::DimDiametric(dimension, leader) {

}


CADEntity_CSPtr DimDiametric::move(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimDiametric = std::make_shared<DimDiametric>(newDimension, this->leader(), layer());
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::copy(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimDiametric = std::make_shared<DimDiametric>(newDimension, this->leader(), layer());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = Dimension(this->definition_point().rotate(rotation_center, rotation_angle),
                                  this->middle_of_text().rotate(rotation_center, rotation_angle),
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), Math::correctAngle(this->angle() + rotation_angle));
    auto newDimDiametric = std::make_shared<DimDiametric>(newDimension, this->leader(), layer());
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimension = Dimension(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
                                  this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimDiametric = std::make_shared<DimDiametric>(newDimension, this->leader(), layer());
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

const geo::Area DimDiametric::boundingBox() const {
    // TODO create bounding box for DimDiametric
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}

Quadratic_CSPtr DimDiametric::quadratic_equation() const {

}
