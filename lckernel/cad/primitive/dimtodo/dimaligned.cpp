
#include "cad/primitive/dimaligned.h"

using namespace lc;

DimAligned::DimAligned(const Dimension& dimension,
                       const geo::Coordinate& extension_point1,
                       const geo::Coordinate& extension_point2,
                       const Layer_CSPtr layer): CADEntity(layer), geo::DimAligned(dimension, extension_point1, extension_point2) {

}

DimAligned::DimAligned(const Dimension& dimension,
                       const geo::Coordinate& extension_point1,
                       const geo::Coordinate& extension_point2,
                       const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo): CADEntity(layer), geo::DimAligned(dimension, extension_point1, extension_point2) {

}


CADEntity_CSPtr DimAligned::move(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimAligned = std::make_shared<DimAligned>(newDimension, this->extension_point1() + offset,
                                                      this->extension_point2() + offset, layer());
    newDimAligned->setID(this->id());
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::copy(const geo::Coordinate& offset) const {
    auto newDimension = Dimension(this->definition_point() + offset, this->middle_of_text() + offset,
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimAligned = std::make_shared<DimAligned>(newDimension, this->extension_point1() + offset,
                                                      this->extension_point2() + offset, layer());
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = Dimension(this->definition_point().rotate(rotation_center, rotation_angle),
                                  this->middle_of_text().rotate(rotation_center, rotation_angle),
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), Math::correctAngle(this->angle() + rotation_angle));
    auto newDimAligned = std::make_shared<DimAligned>(newDimension, this->extension_point1().rotate(rotation_center, rotation_angle), this->extension_point2().rotate(rotation_center, rotation_angle), layer());
    newDimAligned->setID(this->id());
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimension = Dimension(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
                                  this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());
    auto newDimAligned = std::make_shared<DimAligned>(newDimension, this->extension_point1().scale(scale_center, scale_factor), this->extension_point2().scale(scale_center, scale_factor), layer());
    newDimAligned->setID(this->id());
    return newDimAligned;
}

const geo::Area DimAligned::boundingBox() const {
    // TODO create bounding box for DimAligned
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}


CADEntity_CSPtr DimAligned::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newDimension = Dimension(this->definition_point(), this->middle_of_text(),
                                  this->valign(), this->halign(),
                                  this->lineSpacingStyle(), this->lineSpacingFactor(),
                                  this->text_value(), this->style(), this->angle());

    auto newDimAligned = std::make_shared<DimAligned>(newDimension, this->extension_point1(), this->extension_point2(), layer, metaInfo);
    newDimAligned->setID(this->id());
    return newDimAligned;

}