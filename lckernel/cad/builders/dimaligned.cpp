#include "dimaligned.h"
#include <cad/primitive/dimaligned.h>

const lc::geo::Coordinate& lc::builder::DimAlignedBuilder::definitionPoint2() const {
    return _definitionPoint2;
}

void lc::builder::DimAlignedBuilder::setDefinitionPoint2(const lc::geo::Coordinate& definitionPoint2) {
    _definitionPoint2 = definitionPoint2;
}

const lc::geo::Coordinate& lc::builder::DimAlignedBuilder::definitionPoint3() const {
    return _definitionPoint3;
}

void lc::builder::DimAlignedBuilder::setDefinitionPoint3(const lc::geo::Coordinate& definitionPoint3) {
    _definitionPoint3 = definitionPoint3;
}

void lc::builder::DimAlignedBuilder::dimAuto(lc::geo::Coordinate p2,
                                             lc::geo::Coordinate p3,
                                             lc::geo::Coordinate middleOfText) {

    auto nearestPoint = geo::Vector(p2, p3).nearestPointOnPath(middleOfText);
    auto distance = nearestPoint.distanceTo(middleOfText);

    geo::Coordinate dir;
    if(((p3.x() - p2.x()) * (middleOfText.y() - p2.y()) - (p3.y() - p2.y()) * (middleOfText.x() - p2.x())) >= 0) {
        dir = (p3 - p2).rotate(0.5 * M_PI);
    }
    else {
        dir = (p3 - p2).rotate(-0.5 * M_PI);
    }

    _definitionPoint = p3.move(dir, distance);
    _definitionPoint2 = p2;
    _definitionPoint3 = p3;
    _middleOfText = middleOfText;
    _attachmentPoint = TextConst::AttachmentPoint::Top_center;
    _textAngle = 0;
    _lineSpacingFactor = 0;
    _lineSpacingStyle = TextConst::LineSpacingStyle::AtLeast;
    _explicitValue = "<>";
}

lc::entity::DimAligned_CSPtr lc::builder::DimAlignedBuilder::build() {
    return lc::entity::DimAligned_CSPtr(new lc::entity::DimAligned(*this));
}
