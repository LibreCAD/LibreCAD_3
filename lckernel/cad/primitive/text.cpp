#include "text.h"


using namespace lc;

Text::Text(const geo::Coordinate& insertion_point,
           const std::string text_value,
           const double height,
           const double angle,
           const std::string style,
           const TextConst::DrawingDirection textgeneration,
           const TextConst::HAlign halign,
           const TextConst::VAlign valign,
           const Layer_CSPtr layer) : CADEntity(layer),
    _insertion_point(insertion_point),
    _text_value(text_value),
    _height(height),
    _angle(angle),
    _style(style),
    _textgeneration(textgeneration),
    _valign(valign),
    _halign(halign) {
}

Text::Text(const geo::Coordinate& insertion_point,
           const std::string text_value,
           const double height,
           const double angle,
           const std::string style,
           const TextConst::DrawingDirection textgeneration,
           const TextConst::HAlign halign,
           const TextConst::VAlign valign,
           const Layer_CSPtr layer,
           const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo),
    _insertion_point(insertion_point),
    _text_value(text_value),
    _height(height),
    _angle(angle),
    _style(style),
    _textgeneration(textgeneration),
    _valign(valign),
    _halign(halign) {
}

Text::Text(const Text_CSPtr& other, bool sameID) : CADEntity(other, sameID),
    _insertion_point(other->_insertion_point),
    _text_value(other->_text_value),
    _height(other->_height),
    _angle(other->_angle),
    _style(other->_style),
    _textgeneration(other->_textgeneration),
    _valign(other->_valign),
    _halign(other->_halign) {
}

CADEntity_CSPtr Text::move(const geo::Coordinate& offset) const {
    auto newText = std::make_shared<Text>(this->_insertion_point + offset,
                                          this->_text_value,
                                          this->_height,
                                          this->_angle,
                                          this->_style,
                                          this->_textgeneration,
                                          this->_halign,
                                          this->_valign,
                                          layer(),
                                          metaInfo());
    newText->setID(this->id());
    return newText;
}

CADEntity_CSPtr Text::copy(const geo::Coordinate& offset) const {
    auto newText = std::make_shared<Text>(
                       this->_insertion_point + offset,
                       this->_text_value,
                       this->_height,
                       this->_angle,
                       this->_style,
                       this->_textgeneration,
                       this->_halign,
                       this->_valign,
                       layer(),
                       metaInfo());
    newText->setID(this->id());
    return newText;
}

CADEntity_CSPtr Text::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newText = std::make_shared<Text>(
                       this->_insertion_point.rotate(rotation_center, rotation_angle),
                       this->_text_value,
                       this->_height,
                       this->_angle,
                       this->_style,
                       this->_textgeneration,
                       this->_halign,
                       this->_valign,
                       layer(),
                       metaInfo());
    return newText;
}

CADEntity_CSPtr Text::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newText = std::make_shared<Text>(
                       this->_insertion_point.scale(scale_center, scale_factor),
                       this->_text_value,
                       this->_height * std::sqrt(scale_factor.x() * scale_factor.y()),  // Does this make sense?
                       this->_angle,
                       this->_style,
                       this->_textgeneration,
                       this->_halign,
                       this->_valign,
                       this->layer(),
                       this->metaInfo());
    newText->setID(this->id());
    return newText;
}

const geo::Area Text::boundingBox() const {
    // TODO create bounding box for text
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}

CADEntity_CSPtr Text::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newText = std::make_shared<Text>(
                       this->_insertion_point,
                       this->_text_value,
                       this->_height,
                       this->_angle,
                       this->_style,
                       this->_textgeneration,
                       this->_halign,
                       this->_valign,
                       this->layer(),
                       this->metaInfo());
    newText->setID(this->id());
    return newText;
}
