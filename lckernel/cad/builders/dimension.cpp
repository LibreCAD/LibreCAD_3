#include "dimension.h"

const lc::geo::Coordinate& lc::builder::DimensionBuilder::definitionPoint() const {
    return _definitionPoint;
}

void lc::builder::DimensionBuilder::setDefinitionPoint(const lc::geo::Coordinate& definitionPoint) {
    _definitionPoint = definitionPoint;
}

const lc::geo::Coordinate& lc::builder::DimensionBuilder::middleOfText() const {
    return _middleOfText;
}

void lc::builder::DimensionBuilder::setMiddleOfText(const lc::geo::Coordinate& middleOfText) {
    _middleOfText = middleOfText;
}

lc::TextConst::AttachmentPoint lc::builder::DimensionBuilder::attachmentPoint() const {
    return _attachmentPoint;
}

void lc::builder::DimensionBuilder::setAttachmentPoint(lc::TextConst::AttachmentPoint attachmentPoint) {
    _attachmentPoint = attachmentPoint;
}

double lc::builder::DimensionBuilder::textAngle() const {
    return _textAngle;
}

void lc::builder::DimensionBuilder::setTextAngle(double textAngle) {
    _textAngle = textAngle;
}

double lc::builder::DimensionBuilder::lineSpacingFactor() const {
    return _lineSpacingFactor;
}

void lc::builder::DimensionBuilder::setLineSpacingFactor(double lineSpacingFactor) {
    _lineSpacingFactor = lineSpacingFactor;
}

lc::TextConst::LineSpacingStyle lc::builder::DimensionBuilder::lineSpacingStyle() const {
    return _lineSpacingStyle;
}

void lc::builder::DimensionBuilder::setLineSpacingStyle(lc::TextConst::LineSpacingStyle lineSpacingStyle) {
    _lineSpacingStyle = lineSpacingStyle;
}

const std::string& lc::builder::DimensionBuilder::explicitValue() const {
    return _explicitValue;
}

void lc::builder::DimensionBuilder::setExplicitValue(const std::string& explicitValue) {
    _explicitValue = explicitValue;
}
