#pragma once

#include <cad/primitive/textconst.h>
#include "cadentity.h"

namespace lc {
namespace builder {
class DimensionBuilder : public CADEntityBuilder {
protected:
    DimensionBuilder();

public:
    const geo::Coordinate& definitionPoint() const;
    void setDefinitionPoint(const geo::Coordinate& definitionPoint);

    const geo::Coordinate& middleOfText() const;
    void setMiddleOfText(const geo::Coordinate& middleOfText);

    TextConst::AttachmentPoint attachmentPoint() const;
    void setAttachmentPoint(TextConst::AttachmentPoint attachmentPoint);

    double textAngle() const;
    void setTextAngle(double textAngle);

    double lineSpacingFactor() const;
    void setLineSpacingFactor(double lineSpacingFactor);

    TextConst::LineSpacingStyle lineSpacingStyle() const;
    void setLineSpacingStyle(TextConst::LineSpacingStyle lineSpacingStyle);

    const std::string& explicitValue() const;
    void setExplicitValue(const std::string& explicitValue);

protected:
    geo::Coordinate _definitionPoint;
    geo::Coordinate _middleOfText;
    TextConst::AttachmentPoint _attachmentPoint;
    double _textAngle;
    double _lineSpacingFactor;
    TextConst::LineSpacingStyle _lineSpacingStyle;
    std::string _explicitValue;
};
}
}