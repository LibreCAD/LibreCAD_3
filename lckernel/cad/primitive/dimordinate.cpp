#include <map>
#include "cad/primitive/dimordinate.h"

using namespace lc;
using namespace entity;

DimOrdinate::DimOrdinate(geo::Coordinate definitionPoint,
                         geo::Coordinate middleOfText,
                         TextConst::AttachmentPoint attachmentPoint,
                         double textAngle,
                         double lineSpacingFactor,
                         TextConst::LineSpacingStyle lineSpacingStyle,
                         std::string explicitValue,
                         geo::Coordinate featurePoint,
                         geo::Coordinate leaderEndPoint,
                         bool xType,
                         meta::Layer_CSPtr layer,
                         meta::MetaInfo_CSPtr metaInfo,
                         meta::Block_CSPtr block):
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    Dimension(std::move(definitionPoint),
              std::move(middleOfText),
              attachmentPoint,
              textAngle,
              lineSpacingFactor,
              lineSpacingStyle,
              std::move(explicitValue)
             ),
    _featurePoint(std::move(featurePoint)),
    _leaderEndPoint(std::move(leaderEndPoint)),
    _xType(xType) {
}

DimOrdinate::DimOrdinate(const DimOrdinate_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    Dimension(*other),
    _featurePoint(other->_featurePoint),
    _leaderEndPoint(other->_leaderEndPoint),
    _xType(other->_xType) {
}

DimOrdinate::DimOrdinate(const lc::builder::DimOrdinateBuilder& builder) :
    CADEntity(builder),
    Dimension(builder),
    _featurePoint(builder.featurePoint()),
    _leaderEndPoint(builder.leaderEndPoint()),
    _xType(builder.xType()) {
}

CADEntity_CSPtr DimOrdinate::move(const geo::Coordinate& offset) const {
    auto newDimOrdinate = std::make_shared<DimOrdinate>(this->definitionPoint() + offset,
                          this->middleOfText() + offset,
                          this->attachmentPoint(),
                          this->textAngle(),
                          this->lineSpacingFactor(),
                          this->lineSpacingStyle(),
                          this->explicitValue(),
                          this->_featurePoint + offset,
                          this->_leaderEndPoint + offset,
                          this->_xType,
                          this->layer(), metaInfo(), block()
                                                       );
    newDimOrdinate->setID(this->id());
    return newDimOrdinate;
}

CADEntity_CSPtr DimOrdinate::copy(const geo::Coordinate& offset) const {
    auto newDimOrdinate = std::make_shared<DimOrdinate>(this->definitionPoint() + offset,
                          this->middleOfText() + offset,
                          this->attachmentPoint(),
                          this->textAngle(),
                          this->lineSpacingFactor(),
                          this->lineSpacingStyle(),
                          this->explicitValue(),
                          this->_featurePoint + offset,
                          this->_leaderEndPoint + offset,
                          this->_xType,
                          this->layer(), metaInfo(), block()
                                                       );
    return newDimOrdinate;
}

CADEntity_CSPtr DimOrdinate::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    auto newDimOrdinate = std::make_shared<DimOrdinate>(this->definitionPoint().rotate(rotation_center, rotation_angle),
                          this->middleOfText().rotate(rotation_center, rotation_angle),
                          this->attachmentPoint(),
                          this->textAngle(),
                          this->lineSpacingFactor(),
                          this->lineSpacingStyle(),
                          this->explicitValue(),
                          this->_featurePoint.rotate(rotation_center, rotation_angle),
                          this->_leaderEndPoint.rotate(rotation_center, rotation_angle),
                          this->_xType,
                          this->layer(), metaInfo(), block()
                                                       );
    newDimOrdinate->setID(this->id());
    return newDimOrdinate;
}

CADEntity_CSPtr DimOrdinate::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimOrdinate = std::make_shared<DimOrdinate>(this->definitionPoint().scale(scale_center, scale_factor),
                          this->middleOfText().scale(scale_center, scale_factor),
                          this->attachmentPoint(),
                          this->textAngle(),
                          this->lineSpacingFactor(),
                          this->lineSpacingStyle(),
                          this->explicitValue(),
                          this->_featurePoint.scale(scale_center, scale_factor),
                          this->_leaderEndPoint.scale(scale_center, scale_factor),
                          this->_xType,
                          this->layer(), metaInfo(), block()
                                                       );
    newDimOrdinate->setID(this->id());
    return newDimOrdinate;
}

CADEntity_CSPtr DimOrdinate::mirror(const geo::Coordinate& axis1,
                                    const geo::Coordinate& axis2) const {
    auto newDimOrdinate = std::make_shared<DimOrdinate>(this->definitionPoint().mirror(axis1, axis2),
                          this->middleOfText().mirror(axis1, axis2),
                          this->attachmentPoint(),
                          this->textAngle(),
                          this->lineSpacingFactor(),
                          this->lineSpacingStyle(),
                          this->explicitValue(),
                          this->_featurePoint.mirror(axis1, axis2),
                          this->_leaderEndPoint.mirror(axis1, axis2),
                          this->_xType,
                          this->layer(), metaInfo(), block()
                                                       );
    newDimOrdinate->setID(this->id());
    return newDimOrdinate;
}

const geo::Area DimOrdinate::boundingBox() const {
    return geo::Area(_featurePoint, _leaderEndPoint).merge(middleOfText());
}

CADEntity_CSPtr DimOrdinate::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newDimOrdinate = std::make_shared<DimOrdinate>(
                              this->definitionPoint(),
                              this->middleOfText(),
                              this->attachmentPoint(),
                              this->textAngle(),
                              this->lineSpacingFactor(),
                              this->lineSpacingStyle(),
                              this->explicitValue(),
                              this->_featurePoint,
                              this->_leaderEndPoint,
                              this->_xType,
                              layer,
                              metaInfo,
                              block
                          );

    newDimOrdinate->setID(this->id());
    return newDimOrdinate;
}

const geo::Coordinate& DimOrdinate::featurePoint() const noexcept {
    return _featurePoint;
}

const geo::Coordinate& DimOrdinate::leaderEndPoint() const noexcept {
    return _leaderEndPoint;
}

bool DimOrdinate::xType() const noexcept {
    return _xType;
}

double DimOrdinate::value() const {
    return _xType ? _featurePoint.x() - definitionPoint().x()
                  : _featurePoint.y() - definitionPoint().y();
}

std::map<unsigned int, geo::Coordinate> DimOrdinate::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;

    dragPoints[0] = definitionPoint();
    dragPoints[1] = middleOfText();
    dragPoints[2] = _featurePoint;
    dragPoints[3] = _leaderEndPoint;

    return dragPoints;
}

CADEntity_CSPtr DimOrdinate::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<DimOrdinate>(dragPoints.at(0),
                         dragPoints.at(1),
                         attachmentPoint(),
                         textAngle(),
                         lineSpacingFactor(),
                         lineSpacingStyle(),
                         explicitValue(),
                         dragPoints.at(2),
                         dragPoints.at(3),
                         _xType,
                         layer(), metaInfo(), block());
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

PropertiesMap DimOrdinate::availableProperties() const {
    PropertiesMap propertyValues;

    getDimensionProperties(propertyValues);
    propertyValues["featurePoint"] = this->featurePoint();
    propertyValues["leaderEndPoint"] = this->leaderEndPoint();
    propertyValues["xType"] = this->xType();

    return propertyValues;
}

CADEntity_CSPtr DimOrdinate::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate definitionPointp, middleOfTextp;
    double textAnglep, lineSpacingFactorp;
    std::string explicitValuep;
    lc::geo::Coordinate featurePointp = this->featurePoint();
    lc::geo::Coordinate leaderEndPointp = this->leaderEndPoint();
    bool xTypep = this->xType();

    setDimensionProperties(propertiesMap, definitionPointp, middleOfTextp, textAnglep, lineSpacingFactorp, explicitValuep);

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
        if (iter->first == "featurePoint") {
            featurePointp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "leaderEndPoint") {
            leaderEndPointp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "xType") {
            xTypep = boost::get<bool>(iter->second);
        }
    }

    auto newDimOrdinate = std::make_shared<DimOrdinate>(definitionPointp, middleOfTextp, attachmentPoint(), textAnglep, lineSpacingFactorp,
                          lineSpacingStyle(), explicitValuep, featurePointp, leaderEndPointp, xTypep,
                          layer(), metaInfo(), block());
    newDimOrdinate->setID(this->id());
    return newDimOrdinate;
}
