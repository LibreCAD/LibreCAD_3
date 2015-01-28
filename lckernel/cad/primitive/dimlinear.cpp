#include "cad/primitive/dimlinear.h"

using namespace lc;

DimLinear::DimLinear(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double textAngle, double const lineSpacingFactor,
                     TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                     geo::Coordinate const& definitionPoint2, geo::Coordinate const& definitionPoint3, const double angle, const double oblique, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo):
        CADEntity(layer, metaInfo), Dimension(definitionPoint, middleOfText, attachmentPoint, textAngle, lineSpacingFactor, lineSpacingStyle, explicitValue),
        _angle(angle), _oblique(oblique), _definitionPoint2(definitionPoint2), _definitionPoint3(definitionPoint3) {

}


DimLinear::DimLinear(const DimLinear_CSPtr other, bool sameID) : CADEntity(other, sameID), Dimension(*other), _angle(other->_angle), _oblique(other->_oblique), _definitionPoint2(other->_definitionPoint2), _definitionPoint3(other->_definitionPoint3) {
}

DimLinear_SPtr DimLinear::dimAuto(geo::Coordinate const& p2, geo::Coordinate const& p3, const double textOffset, std::string const& explicitValue,const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) {

    const double dx = std::abs(p2.x() - p3.x());
    const double dy = std::abs(p2.y() - p3.y());
    const bool isHorizontal = dx > dy;

    geo::Coordinate p0;
    geo::Coordinate pp2;
    geo::Coordinate pp3;

    geo::Coordinate middletext(p2.mid(p3));
    if (isHorizontal) {
        p0 = geo::Coordinate(p3.x(), p2.mid(p3).y()).move(geo::Coordinate(0.,1.), textOffset);
        middletext = p2.mid(p3).move(geo::Coordinate(0.,1.), textOffset);
    } else {
        p0 = geo::Coordinate(p3.mid(p2).x(), p3.y()).move(geo::Coordinate(1.,0.), textOffset);
        middletext = p2.mid(p3).move(geo::Coordinate(1.,0.), textOffset);
    }

    return std::make_shared<DimLinear>(p0, middletext, TextConst::AttachmentPoint::Top_center , 0., 0., TextConst::LineSpacingStyle::AtLeast, explicitValue, p2, p3, 0., 0., layer, metaInfo);
}



CADEntity_CSPtr DimLinear::move(const geo::Coordinate& offset) const {
    auto newDimLinear = std::make_shared<DimLinear>(this->definitionPoint() + offset, this->middleOfText() + offset, this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(),  this->_definitionPoint2 + offset,  this->_definitionPoint3 + offset, this->_angle, this->_oblique, this->layer(), this->metaInfo());
    newDimLinear->setID(this->id());
    return newDimLinear;
}

CADEntity_CSPtr DimLinear::copy(const geo::Coordinate& offset) const {
    auto newDimLinear = std::make_shared<DimLinear>(this->definitionPoint() + offset, this->middleOfText() + offset, this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(),  this->_definitionPoint2 + offset, this->_definitionPoint3 + offset, this->_angle, this->_oblique, this->layer(), this->metaInfo());
    return newDimLinear;
}

CADEntity_CSPtr DimLinear::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimLinear = std::make_shared<DimLinear>(this->definitionPoint().rotate(rotation_center, rotation_angle),
                                                    this->middleOfText().rotate(rotation_center, rotation_angle), this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_definitionPoint2.rotate(rotation_center, rotation_angle), this->_definitionPoint3.rotate(rotation_center, rotation_angle), this->_angle, this->_oblique, this->layer(), this->metaInfo());
    return newDimLinear;
}

CADEntity_CSPtr DimLinear::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimLinear = std::make_shared<DimLinear>(this->definitionPoint().scale(scale_center, scale_factor),
                                                    this->middleOfText().scale(scale_center, scale_factor), this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_definitionPoint2.scale(scale_center, scale_factor), this->_definitionPoint3.scale(scale_center, scale_factor), this->_angle, this->_oblique, this->layer(), this->metaInfo());
    return newDimLinear;
}

const geo::Area DimLinear::boundingBox() const {
    // TODO create proper bounding box for DimLinear
    return geo::Area(this->middleOfText(), 0., 0.);
}

CADEntity_CSPtr DimLinear::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newDimLinear = std::make_shared<DimLinear>(
                            this->definitionPoint(), this->middleOfText(), this->attachmentPoint(), this->textAngle(), this->lineSpacingFactor(), this->lineSpacingStyle(), this->explicitValue(), this->_definitionPoint2, this->_definitionPoint3, this->_angle, this->_oblique, this->layer(), metaInfo);
    return newDimLinear;
}

double DimLinear::angle() const {
    return _angle;
}
double DimLinear::oblique() const {
    return _oblique;
}

geo::Coordinate DimLinear::definitionPoint2() const {
    return _definitionPoint2;
}
geo::Coordinate DimLinear::definitionPoint3() const {
    return _definitionPoint3;
}