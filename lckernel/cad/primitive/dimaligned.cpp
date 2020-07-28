#include <map>
#include "cad/primitive/dimaligned.h"
#include "dimaligned.h"


using namespace lc;
using namespace entity;

DimAligned::DimAligned(geo::Coordinate definitionPoint,
                       geo::Coordinate middleOfText,
                       TextConst::AttachmentPoint attachmentPoint,
                       double textAngle,
                       double lineSpacingFactor,
                       TextConst::LineSpacingStyle lineSpacingStyle,
                       std::string explicitValue,
                       geo::Coordinate definitionPoint2,
                       geo::Coordinate definitionPoint3,
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
    _definitionPoint2(std::move(definitionPoint2)),
    _definitionPoint3(std::move(definitionPoint3)) {

}


DimAligned::DimAligned(const DimAligned_CSPtr& other, bool sameID) :
        CADEntity(other, sameID),
        Dimension(*other),
        _definitionPoint2(other->_definitionPoint2),
        _definitionPoint3(other->_definitionPoint3) {
}

DimAligned::DimAligned(const lc::builder::DimAlignedBuilder& builder) :
        CADEntity(builder),
        Dimension(builder),
        _definitionPoint2(builder.definitionPoint2()),
        _definitionPoint3(builder.definitionPoint3()) {

}

CADEntity_CSPtr DimAligned::move(const geo::Coordinate& offset) const {
    auto newDimAligned = std::make_shared<DimAligned>(this->definitionPoint() + offset,
                                                      this->middleOfText() + offset,
                                                      this->attachmentPoint(),
                                                      this->textAngle(),
                                                      this->lineSpacingFactor(),
                                                      this->lineSpacingStyle(),
                                                      this->explicitValue(),
                                                      this->_definitionPoint2 + offset,
                                                      this->_definitionPoint3 + offset,
                                                      this->layer(), metaInfo(), block()
    );
    newDimAligned->setID(this->id());
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::copy(const geo::Coordinate& offset) const {
    auto newDimAligned = std::make_shared<DimAligned>(this->definitionPoint() + offset,
                                                      this->middleOfText() + offset,
                                                      this->attachmentPoint(),
                                                      this->textAngle(),
                                                      this->lineSpacingFactor(),
                                                      this->lineSpacingStyle(),
                                                      this->explicitValue(),
                                                      this->_definitionPoint2 + offset,
                                                      this->_definitionPoint3 + offset,
                                                      this->layer(), metaInfo(), block()
    );
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    auto newDimAligned = std::make_shared<DimAligned>(this->definitionPoint().rotate(rotation_center, rotation_angle),
                                                      this->middleOfText().rotate(rotation_center, rotation_angle),
                                                      this->attachmentPoint(),
                                                      this->textAngle(),
                                                      this->lineSpacingFactor(),
                                                      this->lineSpacingStyle(),
                                                      this->explicitValue(),
                                                      this->_definitionPoint2.rotate(rotation_center, rotation_angle),
                                                      this->_definitionPoint3.rotate(rotation_center, rotation_angle),
                                                      this->layer(), metaInfo(), block()
    );
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimAligned = std::make_shared<DimAligned>(this->definitionPoint().scale(scale_center, scale_factor),
                                                      this->middleOfText().scale(scale_center, scale_factor),
                                                      this->attachmentPoint(),
                                                      this->textAngle(),
                                                      this->lineSpacingFactor(),
                                                      this->lineSpacingStyle(),
                                                      this->explicitValue(),
                                                      this->_definitionPoint2.scale(scale_center, scale_factor),
                                                      this->_definitionPoint3.scale(scale_center, scale_factor),
                                                      this->layer(), metaInfo(), block()
    );
    return newDimAligned;
}

CADEntity_CSPtr DimAligned::mirror(const geo::Coordinate& axis1,
                    const geo::Coordinate& axis2) const {

    auto newDimAligned = std::make_shared<DimAligned>(this->definitionPoint().mirror(axis1, axis2),
                                                      this->middleOfText().mirror(axis1, axis2),
                                                      this->attachmentPoint(),
                                                      this->textAngle(),
                                                      this->lineSpacingFactor(),
                                                      this->lineSpacingStyle(),
                                                      this->explicitValue(),
                                                      this->_definitionPoint2.mirror(axis1, axis2),
                                                      this->_definitionPoint3.mirror(axis1, axis2),
                                                      this->layer(), metaInfo(), block()
    );
    return newDimAligned;
}

const geo::Area DimAligned::boundingBox() const {
    // TODO create proper bounding box for DimAligned
    return geo::Area(this->middleOfText(), 0., 0.);
}

CADEntity_CSPtr DimAligned::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newDimAligned = std::make_shared<DimAligned>(
                             this->definitionPoint(),
                             this->middleOfText(),
                             this->attachmentPoint(),
                             this->textAngle(),
                             this->lineSpacingFactor(),
                             this->lineSpacingStyle(),
                             this->explicitValue(),
                             this->_definitionPoint2,
                             this->_definitionPoint3,
                             layer,
                             metaInfo,
                             block
    );

    return newDimAligned;
}

const geo::Coordinate& DimAligned::definitionPoint2() const noexcept {
    return _definitionPoint2;
}
const geo::Coordinate& DimAligned::definitionPoint3() const noexcept {
    return _definitionPoint3;
}

std::map<unsigned int, geo::Coordinate> DimAligned::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;

    dragPoints[0] = definitionPoint();
    dragPoints[1] = middleOfText();
    dragPoints[2] = _definitionPoint2;
    dragPoints[3] = _definitionPoint3;

    return dragPoints;
}


CADEntity_CSPtr DimAligned::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<DimAligned>(dragPoints.at(0),
                                                      dragPoints.at(1),
                                                      attachmentPoint(),
                                                      textAngle(),
                                                      lineSpacingFactor(),
                                                      lineSpacingStyle(),
                                                      explicitValue(),
                                                      dragPoints.at(2),
                                                      dragPoints.at(3),
                                                      layer(), metaInfo(), block());
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

PropertiesMap DimAligned::availableProperties() const {
    PropertiesMap propertyValues;

    getDimensionProperties(propertyValues);
    propertyValues["definitionPoint2"] = this->definitionPoint2();
    propertyValues["definitionPoint3"] = this->definitionPoint3();

    return propertyValues;
}

CADEntity_CSPtr DimAligned::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate definitionPointp, middleOfTextp;
    double textAnglep, lineSpacingFactorp;
    std::string explicitValuep;
    lc::geo::Coordinate definitionPoint2p = this->definitionPoint2();
    lc::geo::Coordinate definitionPoint3p = this->definitionPoint3();

    setDimensionProperties(propertiesMap, definitionPointp, middleOfTextp, textAnglep, lineSpacingFactorp, explicitValuep);

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter)
    {
        if (iter->first == "definitionPoint2") {
            definitionPoint2p = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "definitionPoint3") {
            definitionPoint3p = boost::get<lc::geo::Coordinate>(iter->second);
        }
    }

    auto newDimAligned = std::make_shared<DimAligned>(definitionPointp, middleOfTextp, attachmentPoint(), textAnglep, lineSpacingFactorp,
        lineSpacingStyle(), explicitValuep, definitionPoint2p, definitionPoint3p, layer(), metaInfo(), block());
    newDimAligned->setID(this->id());
    return newDimAligned;
}
