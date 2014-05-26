#include "text.h"



using namespace lc;

Text::Text(const geo::Coordinate& insertion_point,
           const geo::Coordinate& second_point,
           const double height,
           const std::string text_value,
           const double width_rel,
           const double angle,
           const std::string style,
           const TextGeneration textgeneration,
           const HAlign halign, const VAlign valign, const Layer_CSPtr layer) : CADEntity(layer), geo::Text(insertion_point, second_point, height,
                       text_value, width_rel, angle, style,
                       textgeneration, halign, valign) {

}

Text::Text(const geo::Coordinate& insertion_point,
           const geo::Coordinate& second_point,
           const double height,
           const std::string text_value,
           const double width_rel,
           const double angle,
           const std::string style,
           const TextGeneration textgeneration,
           const HAlign halign, const VAlign valign, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer, metaTypes), geo::Text(insertion_point, second_point, height,
                       text_value, width_rel, angle, style,
                       textgeneration, halign, valign) {

}

CADEntity_CSPtr Text::move(const geo::Coordinate& offset) const {
    auto newText = std::make_shared<Text>(this->insertion_point() + offset,
                                          this->second_point() + offset,
                                          this->height(), this->text_value(),
                                          this->width_rel(), this->angle(), this->style(),
                                          this->textgeneration(), this->halign(), this->valign(), layer());
    newText->setID(this->id());
    return newText;
}

CADEntity_CSPtr Text::copy(const geo::Coordinate& offset) const {
    auto newText = std::make_shared<Text>(this->insertion_point() + offset,
                                          this->second_point() + offset,
                                          this->height(), this->text_value(),
                                          this->width_rel(), this->angle(), this->style(),
                                          this->textgeneration(), this->halign(), this->valign(), layer());
    return newText;

}

CADEntity_CSPtr Text::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    // TODO : Implement the correct angle fuction to get angle between 0 adn 360
    auto newText = std::make_shared<Text>(this->insertion_point().rotate(rotation_center, rotation_angle),
                                          this->second_point().rotate(rotation_center, rotation_angle),
                                          this->height(), this->text_value(),
                                          this->width_rel(), Math::correctAngle(this->angle() + rotation_angle), this->style(),
                                          this->textgeneration(), this->halign(), this->valign(), layer());
    newText->setID(this->id());
    return newText;
}

CADEntity_CSPtr Text::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newText = std::make_shared<Text>(this->insertion_point().scale(scale_center, scale_factor),
                                          this->second_point().scale(scale_center, scale_factor),
                                          this->height() * scale_factor.x(), this->text_value(),
                                          this->width_rel(), this->angle(), this->style(),
                                          this->textgeneration(), this->halign(), this->valign(), layer());
    newText->setID(this->id());
    return newText;
}

const geo::Area Text::boundingBox() const {
    // TODO create bounding box for text
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}
