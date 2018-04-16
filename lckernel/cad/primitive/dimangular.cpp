#include "cad/primitive/dimangular.h"


using namespace lc;
using namespace entity;

DimAngular::DimAngular(
    geo::Coordinate const& definitionPoint,
    geo::Coordinate const& middleOfText,
    TextConst::AttachmentPoint const& attachmentPoint,
    double textAngle,
    double const lineSpacingFactor,
    TextConst::LineSpacingStyle const& lineSpacingStyle,
    std::string const& explicitValue,
    geo::Coordinate const& defLine11,
    geo::Coordinate const& defLine12,
    geo::Coordinate const& defLine21,
    geo::Coordinate const& defLine22,
    Layer_CSPtr layer,
    MetaInfo_CSPtr metaInfo,
    Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        Dimension(definitionPoint, middleOfText, attachmentPoint, textAngle, lineSpacingFactor, lineSpacingStyle, explicitValue),
        _defLine11(defLine11),
        _defLine12(defLine12),
        _defLine21(defLine21),
        _defLine22(defLine22) {

}


DimAngular::DimAngular(const DimAngular_CSPtr other, bool sameID) :
    CADEntity(other, sameID),
    Dimension(*other),
    _defLine11(other->_defLine11),
    _defLine12(other->_defLine12),
    _defLine21(other->_defLine21),
    _defLine22(other->_defLine22) {
}

DimAngular_SPtr DimAngular::dimAuto(
        geo::Coordinate const& center,
        geo::Coordinate const& p1,
        geo::Coordinate const& p2,
        std::string const& explicitValue,
        Layer_CSPtr layer,
        MetaInfo_CSPtr metaInfo,
        Block_CSPtr block) {
    geo::Coordinate middletext(p1.mid(p2));

    return std::make_shared<DimAngular>(center,
                                        middletext,
                                        TextConst::AttachmentPoint::Top_center,
                                        0.,
                                        0.,
                                        TextConst::LineSpacingStyle::AtLeast,
                                        explicitValue,
                                        center,
                                        p1,
                                        center,
                                        p2,
                                        std::move(layer),
                                        std::move(metaInfo),
                                        std::move(block)
    );
}


CADEntity_CSPtr DimAngular::move(const geo::Coordinate& offset) const {
    auto newDimAngular = std::make_shared<DimAngular>(this->definitionPoint() + offset, this->middleOfText() + offset, this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_defLine11 + offset, this->_defLine12 + offset, this->_defLine21 + offset, this->_defLine22 + offset, this->layer(), this->metaInfo());
    newDimAngular->setID(this->id());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::copy(const geo::Coordinate& offset) const {
    auto newDimAngular = std::make_shared<DimAngular>(this->definitionPoint() + offset, this->middleOfText() + offset, this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_defLine11 + offset, this->_defLine12 + offset, this->_defLine21 + offset, this->_defLine22 + offset, this->layer(), this->metaInfo());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimAngular = std::make_shared<DimAngular>(this->definitionPoint().rotate(rotation_center, rotation_angle),
                                                      this->middleOfText().rotate(rotation_center, rotation_angle), this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_defLine11.rotate(rotation_center, rotation_angle), this->_defLine12.rotate(rotation_center, rotation_angle), this->_defLine21.rotate(rotation_center, rotation_angle), this->_defLine22.rotate(rotation_center, rotation_angle), this->layer(), this->metaInfo());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimAngular = std::make_shared<DimAngular>(this->definitionPoint().scale(scale_center, scale_factor),
                                                      this->middleOfText().scale(scale_center, scale_factor), this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_defLine11.scale(scale_center, scale_factor), this->_defLine12.scale(scale_center, scale_factor), this->_defLine21.scale(scale_center, scale_factor), this->_defLine22.scale(scale_center, scale_factor), this->layer(), this->metaInfo());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto newDimAngular = std::make_shared<DimAngular>(this->definitionPoint().mirror(axis1,axis2),
                                                      this->middleOfText().mirror(axis1,axis2), this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_defLine11.mirror(axis1,axis2), this->_defLine12.mirror(axis1,axis2), this->_defLine21.mirror(axis1,axis2), this->_defLine22.mirror(axis1,axis2), this->layer(), this->metaInfo());
    return newDimAngular;
}

const geo::Area DimAngular::boundingBox() const {
    // TODO create proper bounding box for DimAngular
    return geo::Area(this->middleOfText(), 0., 0.);
}

CADEntity_CSPtr DimAngular::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo, Block_CSPtr block) const {
    auto newDimAngular = std::make_shared<DimAngular>(
                             this->definitionPoint(),
                             this->middleOfText(),
                             this->attachmentPoint(),
                             this->textAngle(),
                             this->lineSpacingFactor(),
                             this->lineSpacingStyle(),
                             this->explicitValue(),
                             this->_defLine11,
                             this->_defLine12,
                             this->_defLine21,
                             this->_defLine22,
                             layer,
                             metaInfo,
                             block
    );

    return newDimAngular;
}

geo::Coordinate DimAngular::defLine11() const {
    return _defLine11;
}

geo::Coordinate DimAngular::defLine12() const {
    return _defLine12;
}

geo::Coordinate DimAngular::defLine21() const {
    return _defLine21;
}

geo::Coordinate DimAngular::defLine22() const {
    return _defLine22;
}

std::map<unsigned int, geo::Coordinate> DimAngular::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;

    dragPoints[0] = definitionPoint();
    dragPoints[1] = middleOfText();
    dragPoints[2] = _defLine11;
    dragPoints[3] = _defLine12;
    dragPoints[4] = _defLine21;
    dragPoints[5] = _defLine22;

    return dragPoints;
}


CADEntity_CSPtr DimAngular::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<DimAngular>(dragPoints.at(0),
                                                      dragPoints.at(1),
                                                      attachmentPoint(),
                                                      textAngle(),
                                                      lineSpacingFactor(),
                                                      lineSpacingStyle(),
                                                      explicitValue(),
                                                      dragPoints.at(2),
                                                      dragPoints.at(3),
                                                      dragPoints.at(4),
                                                      dragPoints.at(5),
                                                      layer(),
                                                      metaInfo());
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}
