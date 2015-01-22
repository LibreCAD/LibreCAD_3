#include "cad/math/lcmath.h"
#include "cad/primitive/dimension.h"

using namespace lc;

Dimension::Dimension(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                     TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue)
    : _definitionPoint(definitionPoint),
      _middleOfText(middleOfText),
      _attachmentPoint(attachmentPoint),
      _angle(angle),
      _lineSpacingFactor(lineSpacingFactor),
      _lineSpacingStyle(lineSpacingStyle),
      _explicitValue(explicitValue) {
}

Dimension::Dimension(const geo::Coordinate& definitionPoint,
                     const geo::Coordinate& middleOfText,
                     const TextConst::AttachmentPoint attachmentPoint,
                     const double angle)   : _definitionPoint(definitionPoint),
    _middleOfText(middleOfText),
    _attachmentPoint(attachmentPoint),
    _angle(angle),
    _lineSpacingFactor(1.),
    _lineSpacingStyle(TextConst::LineSpacingStyle::AtLeast),
    _explicitValue("<>")  {
}

Dimension::Dimension(Dimension const& other)  : _definitionPoint(other.definitionPoint()),
    _middleOfText(other.middleOfText()),
    _attachmentPoint(other.attachmentPoint()),
    _angle(other.angle()),
    _lineSpacingFactor(other.lineSpacingFactor()),
    _lineSpacingStyle(other.lineSpacingStyle()),
    _explicitValue(other.explicitValue()) {

}
/*
CADEntity_CSPtr Dimension::move(const Coordinate& offset) const {
    auto newDimension = std::make_shared<Dimension>(this->definitionPoint() + offset, this->middleOfText() + offset,this->attachmentPoint(), this->angle(),
                                                    this->lineSpacingFactor(), this->lineSpacingStyle(), explicitValue());
    return newDimension;
}

CADEntity_CSPtr Dimension::copy(const Coordinate& offset) const {
    auto newDimension = std::make_shared<Dimension>(this->definitionPoint() + offset, this->middleOfText() + offset,this->attachmentPoint(), this->angle(),
            this->lineSpacingFactor(), this->lineSpacingStyle(), explicitValue());
    return newDimension;
}

CADEntity_CSPtr Dimension::rotate(const Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = std::make_shared<Dimension>(this->definitionPoint().rotate(rotation_center, rotation_angle), this->middleOfText().rotate(rotation_center, rotation_angle),
            Math::correctAngle(this->angle() + rotation_angle), this->lineSpacingFactor(), this->lineSpacingStyle(), explicitValue());
    return newDimension;
}

CADEntity_CSPtr Dimension::scale(const Coordinate& scale_center, const Coordinate& scale_factor) const {
    auto newDimension = std::make_shared<Dimension>(this->definitionPoint().scale(scale_center, scale_factor), this->middleOfText().scale(scale_center, scale_factor),
            , this->angle(), this->lineSpacingFactor(), this->lineSpacingStyle(), explicitValue());
    return newDimension;
}

CADEntity_CSPtr Dimension::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newEntity = std::make_shared<Dimension>(this->definitionPoint(), this->middleOfText(),this->attachmentPoint(), this->angle(),
            this->lineSpacingFactor(), this->lineSpacingStyle(), explicitValue());
    return newEntity;
}
*/

geo::Coordinate Dimension::definitionPoint() const {
    return _definitionPoint;
}

geo::Coordinate Dimension::middleOfText() const {
    return _middleOfText;
}

double Dimension::angle() const {
    return _angle;
}

TextConst::AttachmentPoint const& Dimension::attachmentPoint() const {
    return _attachmentPoint;
}

double Dimension::lineSpacingFactor() const {
    return _lineSpacingFactor;
}

TextConst::LineSpacingStyle Dimension::lineSpacingStyle() const {
    return _lineSpacingStyle;
}

std::string Dimension::explicitValue() const {
    return _explicitValue;
}