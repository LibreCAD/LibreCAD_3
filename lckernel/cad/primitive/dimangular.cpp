#include "cad/primitive/dimangular.h"

using namespace lc;

DimAngular::DimAngular(const Dimension& dimension,
                       const geo::Coordinate& definition_point1,
                       const geo::Coordinate& definition_point2,
                       const geo::Coordinate& definition_point3,
                       const geo::Coordinate& definition_point4,
                       const Layer_CSPtr layer): CADEntity(layer), geo::DimAngular(dimension, definition_point1, definition_point2, definition_point3, definition_point4) {

}

DimAngular::DimAngular(const Dimension& dimension,
                       const geo::Coordinate& definition_point1,
                       const geo::Coordinate& definition_point2,
                       const geo::Coordinate& definition_point3,
                       const geo::Coordinate& definition_point4,
                       const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo): CADEntity(layer), geo::DimAngular(dimension, definition_point1, definition_point2, definition_point3, definition_point4) {

}


CADEntity_CSPtr DimAngular::move(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimAngular = std::make_shared<DimAngular>(newDimension, this->definition_point1() + offset,
                                                      this->definition_point2() + offset, this->definition_point3() + offset, this->definition_point4() + offset, layer());
    newDimAngular->setID(this->id());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::copy(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimAngular = std::make_shared<DimAngular>(newDimension, this->definition_point1() + offset,
                                                      this->definition_point2() + offset, this->definition_point3() + offset, this->definition_point4() + offset, layer());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = Dimension(this->definition_point().rotate(rotation_center, rotation_angle),
                                  this->middle_of_text().rotate(rotation_center, rotation_angle),
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), Math::correctAngle(this->angle() + rotation_angle));
    auto newDimAngular = std::make_shared<DimAngular>(newDimension, this->definition_point1().rotate(rotation_center, rotation_angle), this->definition_point2().rotate(rotation_center, rotation_angle), this->definition_point3().rotate(rotation_center, rotation_angle), this->definition_point4().rotate(rotation_center, rotation_angle), layer());
    newDimAngular->setID(this->id());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimension = Dimension(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
                                  this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimAngular = std::make_shared<DimAngular>(newDimension, this->definition_point1().scale(scale_center, scale_factor), this->definition_point2().scale(scale_center, scale_factor), this->definition_point3().scale(scale_center, scale_factor), this->definition_point4().scale(scale_center, scale_factor), layer());
    newDimAngular->setID(this->id());
    return newDimAngular;
}

const geo::Area DimAngular::boundingBox() const {
    // TODO create bounding box for DimAngular
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}

CADEntity_CSPtr DimAngular::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newDimension = Dimension(this->definition_point(), this->middle_of_text(),
            this->valign(), this->halign(),
            this->lineSpacingStyle(), this->lineSpacingFactor(),
            this->text_value(), this->style(), this->angle());
    auto newDimAngular = std::make_shared<DimAngular>(newDimension, this->definition_point1(),
            this->definition_point2(), this->definition_point3(), this->definition_point4(), layer, metaInfo);
    newDimAngular->setID(this->id());
    return newDimAngular;
}
