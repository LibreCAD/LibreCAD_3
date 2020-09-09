#include "cad/math/lcmath.h"
#include "cad/primitive/dimension.h"
#include "dimension.h"


using namespace lc;
using namespace entity;

Dimension::Dimension(geo::Coordinate definitionPoint,
                     geo::Coordinate middleOfText,
                     TextConst::AttachmentPoint attachmentPoint,
                     double textAngle,
                     double lineSpacingFactor,
                     TextConst::LineSpacingStyle lineSpacingStyle,
                     std::string explicitValue) :
    _definitionPoint(std::move(definitionPoint)),
    _middleOfText(std::move(middleOfText)),
    _attachmentPoint(attachmentPoint),
    _textAngle(textAngle),
    _lineSpacingFactor(lineSpacingFactor),
    _lineSpacingStyle(lineSpacingStyle),
    _explicitValue(std::move(explicitValue)) {
}

Dimension::Dimension(geo::Coordinate definitionPoint,
                     geo::Coordinate middleOfText,
                     TextConst::AttachmentPoint attachmentPoint,
                     double textAngle) :
    _definitionPoint(std::move(definitionPoint)),
    _middleOfText(std::move(middleOfText)),
    _attachmentPoint(attachmentPoint),
    _textAngle(textAngle),
    _lineSpacingFactor(1.),
    _lineSpacingStyle(TextConst::LineSpacingStyle::AtLeast),
    _explicitValue("<>")  {
}

Dimension::Dimension(const Dimension& other) :
    _definitionPoint(other.definitionPoint()),
    _middleOfText(other._middleOfText),
    _attachmentPoint(other._attachmentPoint),
    _textAngle(other._textAngle),
    _lineSpacingFactor(other._lineSpacingFactor),
    _lineSpacingStyle(other._lineSpacingStyle),
    _explicitValue(other._explicitValue) {
}

Dimension::Dimension(const builder::DimensionBuilder& builder) :
    _definitionPoint(builder.definitionPoint()),
    _middleOfText(builder.middleOfText()),
    _attachmentPoint(builder.attachmentPoint()),
    _textAngle(builder.textAngle()),
    _lineSpacingFactor(builder.lineSpacingFactor()),
    _lineSpacingStyle(builder.lineSpacingStyle()),
    _explicitValue(builder.explicitValue()) {
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

void Dimension::getDimensionProperties(PropertiesMap& propertyValues) const {
    propertyValues["definitionPoint"] = this->definitionPoint();
    propertyValues["middleOfText"] = this->middleOfText();
    propertyValues["textAngle"] = this->textAngle();
    propertyValues["lineSpacingFactor"] = this->lineSpacingFactor();
    propertyValues["explicitValue"] = this->explicitValue();
}

void Dimension::setDimensionProperties(const PropertiesMap& propertiesMap, lc::geo::Coordinate& definitionPointp, lc::geo::Coordinate& middleOfTextp,
                                       double& textAnglep, double& lineSpacingFactorp, std::string& explicitValuep) const {
    definitionPointp = this->definitionPoint();
    middleOfTextp = this->middleOfText();
    textAnglep = this->textAngle();
    lineSpacingFactorp = this->lineSpacingFactor();
    explicitValuep = this->explicitValue();

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter)
    {
        if (iter->first == "definitionPoint") {
            definitionPointp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "middleOfText") {
            middleOfTextp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "textAngle") {
            textAnglep = boost::get<double>(iter->second);
        }

        if (iter->first == "lineSpacingFactor") {
            lineSpacingFactorp = boost::get<double>(iter->second);
        }

        if (iter->first == "explicitValue") {
            explicitValuep = boost::get<std::string>(iter->second);
        }
    }
}
