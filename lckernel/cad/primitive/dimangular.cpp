#include "cad/primitive/dimangular.h"


using namespace lc;
using namespace entity;

DimAngular::DimAngular(
    geo::Coordinate definitionPoint,
    geo::Coordinate middleOfText,
    TextConst::AttachmentPoint attachmentPoint,
    double textAngle,
    double lineSpacingFactor,
    TextConst::LineSpacingStyle lineSpacingStyle,
    std::string explicitValue,
    geo::Coordinate defLine11,
    geo::Coordinate defLine12,
    geo::Coordinate defLine21,
    geo::Coordinate defLine22,
    meta::Layer_CSPtr layer,
    meta::MetaInfo_CSPtr metaInfo,
    meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        Dimension(std::move(definitionPoint),
                  std::move(middleOfText),
                  attachmentPoint,
                  textAngle,
                  lineSpacingFactor,
                  lineSpacingStyle,
                  std::move(explicitValue)
        ),
        _defLine11(std::move(defLine11)),
        _defLine12(std::move(defLine12)),
        _defLine21(std::move(defLine21)),
        _defLine22(std::move(defLine22)) {

}


DimAngular::DimAngular(const DimAngular_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    Dimension(*other),
    _defLine11(other->_defLine11),
    _defLine12(other->_defLine12),
    _defLine21(other->_defLine21),
    _defLine22(other->_defLine22) {
}

DimAngular_SPtr DimAngular::dimAuto(
        const geo::Coordinate& center,
        geo::Coordinate p1,
        geo::Coordinate p2,
        std::string explicitValue,
        meta::Layer_CSPtr layer,
        meta::MetaInfo_CSPtr metaInfo,
        meta::Block_CSPtr block) {
    geo::Coordinate middletext(p1.mid(p2));

    return std::make_shared<DimAngular>(center,
                                        std::move(middletext),
                                        TextConst::AttachmentPoint::Top_center,
                                        0.,
                                        0.,
                                        TextConst::LineSpacingStyle::AtLeast,
                                        std::move(explicitValue),
                                        center,
                                        std::move(p1),
                                        center,
                                        std::move(p2),
                                        std::move(layer),
                                        std::move(metaInfo),
                                        std::move(block)
    );
}


CADEntity_CSPtr DimAngular::move(const geo::Coordinate& offset) const {
    auto newDimAngular = std::make_shared<DimAngular>(
            definitionPoint() + offset,
            middleOfText() + offset,
            attachmentPoint(),
            textAngle(),
            lineSpacingFactor(),
            lineSpacingStyle(),
            explicitValue(),
            _defLine11 + offset,
            _defLine12 + offset,
            _defLine21 + offset,
            _defLine22 + offset,
            layer(),
            metaInfo()
    );
    newDimAngular->setID(this->id());
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::copy(const geo::Coordinate& offset) const {
    auto newDimAngular = std::make_shared<DimAngular>(
            definitionPoint() + offset,
            middleOfText() + offset,
            attachmentPoint(),
            textAngle(),
            lineSpacingFactor(),
            lineSpacingStyle(),
            explicitValue(),
            _defLine11 + offset,
            _defLine12 + offset,
            _defLine21 + offset,
            _defLine22 + offset,
            layer(),
            metaInfo()
    );
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimAngular = std::make_shared<DimAngular>(
            definitionPoint().rotate(rotation_center, rotation_angle),
            middleOfText().rotate(rotation_center, rotation_angle), 
            attachmentPoint(), 
            textAngle(), 
            lineSpacingFactor(), 
            lineSpacingStyle(), 
            explicitValue(), 
            _defLine11.rotate(rotation_center, rotation_angle), 
            _defLine12.rotate(rotation_center, rotation_angle), 
            _defLine21.rotate(rotation_center, rotation_angle), 
            _defLine22.rotate(rotation_center, rotation_angle), 
            layer(), 
            metaInfo()
    );
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimAngular = std::make_shared<DimAngular>(
            definitionPoint().scale(scale_center, scale_factor),
            middleOfText().scale(scale_center, scale_factor), 
            attachmentPoint(), 
            textAngle(), 
            lineSpacingFactor(), 
            lineSpacingStyle(), 
            explicitValue(), 
            _defLine11.scale(scale_center, scale_factor), 
            _defLine12.scale(scale_center, scale_factor), 
            _defLine21.scale(scale_center, scale_factor), 
            _defLine22.scale(scale_center, scale_factor), 
            layer(), 
            metaInfo()
    );
    return newDimAngular;
}

CADEntity_CSPtr DimAngular::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto newDimAngular = std::make_shared<DimAngular>(
            definitionPoint().mirror(axis1,axis2),
            middleOfText().mirror(axis1,axis2), 
            attachmentPoint(), 
            textAngle(), 
            lineSpacingFactor(), 
            lineSpacingStyle(), 
            explicitValue(), 
            _defLine11.mirror(axis1,axis2), 
            _defLine12.mirror(axis1,axis2), 
            _defLine21.mirror(axis1,axis2), 
            _defLine22.mirror(axis1,axis2), 
            layer(), 
            metaInfo()
    );
    return newDimAngular;
}

const geo::Area DimAngular::boundingBox() const {
    // TODO create proper bounding box for DimAngular
    return geo::Area(this->middleOfText(), 0., 0.);
}

CADEntity_CSPtr DimAngular::modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newDimAngular = std::make_shared<DimAngular>(
        definitionPoint(),
        middleOfText(),
        attachmentPoint(),
        textAngle(),
        lineSpacingFactor(),
        lineSpacingStyle(),
        explicitValue(),
        _defLine11,
        _defLine12,
        _defLine21,
        _defLine22,
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
        auto newEntity = std::make_shared<DimAngular>(
            dragPoints.at(0),
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
            metaInfo()
        );
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}
