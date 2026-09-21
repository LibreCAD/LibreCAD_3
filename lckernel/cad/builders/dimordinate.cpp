#include "dimordinate.h"
#include <cad/primitive/dimordinate.h>

#include <cmath>

lc::builder::DimOrdinateBuilder::DimOrdinateBuilder() :
    _xType(true) {
}

const lc::geo::Coordinate& lc::builder::DimOrdinateBuilder::featurePoint() const {
    return _featurePoint;
}

void lc::builder::DimOrdinateBuilder::setFeaturePoint(const lc::geo::Coordinate& featurePoint) {
    _featurePoint = featurePoint;
}

const lc::geo::Coordinate& lc::builder::DimOrdinateBuilder::leaderEndPoint() const {
    return _leaderEndPoint;
}

void lc::builder::DimOrdinateBuilder::setLeaderEndPoint(const lc::geo::Coordinate& leaderEndPoint) {
    _leaderEndPoint = leaderEndPoint;
}

bool lc::builder::DimOrdinateBuilder::xType() const {
    return _xType;
}

void lc::builder::DimOrdinateBuilder::setXType(bool xType) {
    _xType = xType;
}

bool lc::builder::DimOrdinateBuilder::measuresX(const lc::geo::Coordinate& featurePoint,
                                                const lc::geo::Coordinate& leaderEndPoint) {
    return std::abs(leaderEndPoint.x() - featurePoint.x()) <= std::abs(leaderEndPoint.y() - featurePoint.y());
}

void lc::builder::DimOrdinateBuilder::dimAuto(const lc::geo::Coordinate& featurePoint,
                                              const lc::geo::Coordinate& leaderEndPoint) {
    _featurePoint = featurePoint;
    _leaderEndPoint = leaderEndPoint;
    _xType = measuresX(featurePoint, leaderEndPoint);
    _middleOfText = leaderEndPoint;

    // DXF's meaning: the point of the text that sits at middleOfText. For a
    // leader running up that is the bottom of the text, so the text is above.
    if (_xType) {
        _attachmentPoint = leaderEndPoint.y() >= featurePoint.y()
                           ? TextConst::AttachmentPoint::Bottom_center
                           : TextConst::AttachmentPoint::Top_center;
    }
    else {
        _attachmentPoint = leaderEndPoint.x() >= featurePoint.x()
                           ? TextConst::AttachmentPoint::Middle_left
                           : TextConst::AttachmentPoint::Middle_right;
    }
}

lc::entity::DimOrdinate_CSPtr lc::builder::DimOrdinateBuilder::build() {
    checkValues(true);
    return lc::entity::DimOrdinate_CSPtr(new lc::entity::DimOrdinate(*this));
}

void lc::builder::DimOrdinateBuilder::copy(lc::entity::DimOrdinate_CSPtr entity) {
    CADEntityBuilder::copy(entity);

    setDefinitionPoint(entity->definitionPoint());
    setMiddleOfText(entity->middleOfText());
    setAttachmentPoint(entity->attachmentPoint());
    setTextAngle(entity->textAngle());
    setLineSpacingFactor(entity->lineSpacingFactor());
    setLineSpacingStyle(entity->lineSpacingStyle());
    setExplicitValue(entity->explicitValue());
    setFeaturePoint(entity->featurePoint());
    setLeaderEndPoint(entity->leaderEndPoint());
    setXType(entity->xType());
}
