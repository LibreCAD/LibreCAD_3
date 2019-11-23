#include "cad/primitive/dimdiametric.h"
#include "dimdiametric.h"


using namespace lc;
using namespace entity;

DimDiametric::DimDiametric(geo::Coordinate definitionPoint,
                           geo::Coordinate middleOfText,
                           TextConst::AttachmentPoint attachmentPoint,
                           double angle,
                           double lineSpacingFactor,
                           TextConst::LineSpacingStyle lineSpacingStyle,
                           std::string explicitValue,
                           geo::Coordinate definitionPoint2,
                           double leader,
                           meta::Layer_CSPtr layer,
                           meta::MetaInfo_CSPtr metaInfo,
                           meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        Dimension(std::move(definitionPoint),
                  std::move(middleOfText),
                  attachmentPoint,
                  angle,
                  lineSpacingFactor,
                  lineSpacingStyle,
                  std::move(explicitValue)),
        _leader(leader),
        _definitionPoint2(std::move(definitionPoint2)) {

}

DimDiametric::DimDiametric(const DimDiametric_CSPtr& other, bool sameID) :
        CADEntity(other, sameID),
        Dimension(*other),
        _leader(other->_leader),
        _definitionPoint2(other->definitionPoint2()) {

}


DimDiametric::DimDiametric(const lc::builder::DimDiametricBuilder& builder) :
    CADEntity(builder),
    Dimension(builder),
    _leader(builder.leader()),
    _definitionPoint2(builder.definitionPoint2()) {

}

CADEntity_CSPtr DimDiametric::move(const geo::Coordinate& offset) const {
    auto newDimDiametric = std::make_shared<DimDiametric>(this->definitionPoint() + offset,
                                                          this->middleOfText() + offset,
                                                          this->attachmentPoint(),
                                                          this->textAngle(),
                                                          this->lineSpacingFactor(),
                                                          this->lineSpacingStyle(),
                                                          this->explicitValue(),
                                                          this->_definitionPoint2 + offset,
                                                          this->_leader,
                                                          this->layer(),
                                                          this->metaInfo());
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::copy(const geo::Coordinate& offset) const {
    auto newDimDiametric = std::make_shared<DimDiametric>(this->definitionPoint() + offset,
                                                          this->middleOfText() + offset,
                                                          this->attachmentPoint(),
                                                          this->textAngle(),
                                                          this->lineSpacingFactor(),
                                                          this->lineSpacingStyle(),
                                                          this->explicitValue(),
                                                          this->_definitionPoint2 + offset,
                                                          this->_leader,
                                                          this->layer(),
                                                          this->metaInfo());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimDiametric = std::make_shared<DimDiametric>(
            this->definitionPoint().rotate(rotation_center, rotation_angle),
            this->middleOfText().rotate(rotation_center, rotation_angle),
            this->attachmentPoint(),
            this->textAngle(),
            this->lineSpacingFactor(),
            this->lineSpacingStyle(),
            this->explicitValue(),
            this->_definitionPoint2.rotate(rotation_center, rotation_angle),
            this->_leader,
            this->layer(),
            this->metaInfo());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimDiametric = std::make_shared<DimDiametric>(this->definitionPoint().scale(scale_center, scale_factor),
                                                          this->middleOfText().scale(scale_center, scale_factor),
                                                          this->attachmentPoint(),
                                                          this->textAngle(),
                                                          this->lineSpacingFactor(),
                                                          this->lineSpacingStyle(),
                                                          this->explicitValue(),
                                                          this->_definitionPoint2.scale(scale_center, scale_factor),
                                                          this->_leader,
                                                          this->layer(),
                                                          this->metaInfo());
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

CADEntity_CSPtr DimDiametric::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto newDimDiametric = std::make_shared<DimDiametric>(this->definitionPoint().mirror(axis1, axis2),
                                                          this->middleOfText().mirror(axis1, axis2),
                                                          this->attachmentPoint(),
                                                          this->textAngle(),
                                                          this->lineSpacingFactor(),
                                                          this->lineSpacingStyle(),
                                                          this->explicitValue(),
                                                          this->_definitionPoint2.mirror(axis1, axis2),
                                                          this->_leader,
                                                          this->layer(),
                                                          this->metaInfo());
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

const geo::Area DimDiametric::boundingBox() const {
    // TODO create proper bounding box for DimLinear
    return geo::Area(this->middleOfText(), 0., 0.);
}

CADEntity_CSPtr DimDiametric::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newDimDiametric = std::make_shared<DimDiametric>(
                               this->definitionPoint(),
                               this->middleOfText(),
                               this->attachmentPoint(),
                               this->textAngle(),
                               this->lineSpacingFactor(),
                               this->lineSpacingStyle(),
                               this->explicitValue(),
                               this->_definitionPoint2,
                               this->_leader,
                               layer,
                               metaInfo,
                               block
    );
    newDimDiametric->setID(this->id());
    return newDimDiametric;
}

double DimDiametric::leader() const {
    return _leader;
}

geo::Coordinate DimDiametric::definitionPoint2() const {
    return _definitionPoint2;
}

std::map<unsigned int, geo::Coordinate> DimDiametric::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;

    dragPoints[0] = definitionPoint();
    dragPoints[1] = middleOfText();
    dragPoints[2] = _definitionPoint2;

    return dragPoints;
}


CADEntity_CSPtr DimDiametric::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<DimDiametric>(dragPoints.at(0),
                                                      dragPoints.at(1),
                                                      attachmentPoint(),
                                                      textAngle(),
                                                      lineSpacingFactor(),
                                                      lineSpacingStyle(),
                                                      explicitValue(),
                                                      dragPoints.at(2),
                                                      leader(),
                                                      layer(),
                                                      metaInfo());
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}
