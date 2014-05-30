#include "cad/primitive/dimradial.h"

using namespace lc;

DimRadial::DimRadial(const Dimension& dimension,
                           const double leader,
                           const Layer_CSPtr layer): CADEntity(layer), geo::DimRadial(dimension, leader) {

}

DimRadial::DimRadial(const Dimension& dimension,
                           const double leader,
                           const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes): CADEntity(layer), geo::DimRadial(dimension, leader) {

}


CADEntity_CSPtr DimRadial::move(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimRadial = std::make_shared<DimRadial>(newDimension, this->leader(), layer());
    newDimRadial->setID(this->id());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::copy(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimRadial = std::make_shared<DimRadial>(newDimension, this->leader(), layer());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = Dimension(this->definition_point().rotate(rotation_center, rotation_angle),
                                  this->middle_of_text().rotate(rotation_center, rotation_angle),
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), Math::correctAngle(this->angle() + rotation_angle));
    auto newDimRadial = std::make_shared<DimRadial>(newDimension, this->leader(), layer());
    newDimRadial->setID(this->id());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimension = Dimension(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
                                  this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimRadial = std::make_shared<DimRadial>(newDimension, this->leader(), layer());
    newDimRadial->setID(this->id());
    return newDimRadial;
}

const geo::Area DimRadial::boundingBox() const {
    // TODO create bounding box for DimRadial
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}
