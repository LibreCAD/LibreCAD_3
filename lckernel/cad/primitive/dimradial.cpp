#include "cad/primitive/dimradial.h"

using namespace lc;

DimRadial::DimRadial(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                     TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                     geo::Coordinate const& definitionPoint2,
                     const double leader, const Layer_CSPtr layer): CADEntity(layer), Dimension(definitionPoint, middleOfText, attachmentPoint, angle, lineSpacingFactor, lineSpacingStyle, explicitValue),
    _leader(leader), _definitionPoint2(definitionPoint2) {

}

DimRadial::DimRadial(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                     TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                     geo::Coordinate const& definitionPoint2,
                     const double leader, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo): CADEntity(layer, metaInfo), Dimension(definitionPoint, middleOfText, attachmentPoint, angle, lineSpacingFactor, lineSpacingStyle, explicitValue),
    _leader(leader), _definitionPoint2(definitionPoint2) {

}

DimRadial::DimRadial(const DimRadial_CSPtr other, bool sameID) : CADEntity(other, sameID), Dimension(*other), _leader(other->_leader), _definitionPoint2(other->definitionPoint2()) {

}

CADEntity_CSPtr DimRadial::move(const geo::Coordinate& offset) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint() + offset, this->middleOfText() + offset, this->attachmentPoint(), this->angle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(),  this->_definitionPoint2 + offset, this->_leader, this->layer(), this->metaInfo());
    newDimRadial->setID(this->id());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::copy(const geo::Coordinate& offset) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint() + offset, this->middleOfText() + offset, this->attachmentPoint(), this->angle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(),  this->_definitionPoint2 + offset, this->_leader, this->layer(), this->metaInfo());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint().rotate(rotation_center, rotation_angle),
                                                    this->middleOfText().rotate(rotation_center, rotation_angle), this->attachmentPoint(), this->angle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_definitionPoint2.rotate(rotation_center, rotation_angle), this->_leader, this->layer(), this->metaInfo());
    return newDimRadial;
}

CADEntity_CSPtr DimRadial::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimRadial = std::make_shared<DimRadial>(this->definitionPoint().scale(scale_center, scale_factor),
                                                    this->middleOfText().scale(scale_center, scale_factor), this->attachmentPoint(), this->angle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_definitionPoint2.scale(scale_center, scale_factor), this->_leader, this->layer(), this->metaInfo());
    return newDimRadial;
}

const geo::Area DimRadial::boundingBox() const {
    // TODO create proper bounding box for DimLinear
    return geo::Area(this->middleOfText(), 0., 0.);
}

CADEntity_CSPtr DimRadial::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newDimRadial = std::make_shared<DimRadial>(
                            this->definitionPoint(), this->middleOfText(), this->attachmentPoint(), this->angle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_definitionPoint2, this->_leader, this->layer(), metaInfo);
    return newDimRadial;
}

double DimRadial::leader() const {
    return _leader;
}

geo::Coordinate DimRadial::definitionPoint2() const {
    return _definitionPoint2;
}