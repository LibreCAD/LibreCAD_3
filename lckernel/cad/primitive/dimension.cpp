#include "cad/math/lcmath.h"
#include "cad/primitive/dimension.h"

using namespace lc;
using namespace entity;

Dimension::Dimension(geo::Coordinate const& definitionPoint,
                     geo::Coordinate const& middleOfText,
                     TextConst::AttachmentPoint const& attachmentPoint,
                     double textAngle,
                     double lineSpacingFactor,
                     TextConst::LineSpacingStyle const& lineSpacingStyle,
                     std::string const& explicitValue) :
        _definitionPoint(definitionPoint),
        _middleOfText(middleOfText),
        _attachmentPoint(attachmentPoint),
        _textAngle(textAngle),
        _lineSpacingFactor(lineSpacingFactor),
        _lineSpacingStyle(lineSpacingStyle),
        _explicitValue(explicitValue) {
}

Dimension::Dimension(const geo::Coordinate& definitionPoint,
                     const geo::Coordinate& middleOfText,
                     const TextConst::AttachmentPoint attachmentPoint,
                     const double textAngle) :
        _definitionPoint(definitionPoint),
        _middleOfText(middleOfText),
        _attachmentPoint(attachmentPoint),
        _textAngle(textAngle),
        _lineSpacingFactor(1.),
        _lineSpacingStyle(TextConst::LineSpacingStyle::AtLeast),
        _explicitValue("<>")  {
}

Dimension::Dimension(Dimension const& other) :
        _definitionPoint(other.definitionPoint()),
        _middleOfText(other._middleOfText),
        _attachmentPoint(other._attachmentPoint),
        _textAngle(other._textAngle),
        _lineSpacingFactor(other._lineSpacingFactor),
        _lineSpacingStyle(other._lineSpacingStyle),
        _explicitValue(other._explicitValue) {

}

const geo::Coordinate& Dimension::definitionPoint() const noexcept {
    return _definitionPoint;
}

const geo::Coordinate& Dimension::middleOfText() const noexcept {
    return _middleOfText;
}

double Dimension::textAngle() const {
    return _textAngle;
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