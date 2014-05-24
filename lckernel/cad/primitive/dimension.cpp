#include "cad/primitive/dimension.h"

using namespace lc;

Dimension::Dimension(const geo::Coordinate& definition_point,
                     const geo::Coordinate& middle_of_text,
                     const geo::MText::VAlign valign,
                     const geo::MText::HAlign halign,
                     const geo::MText::LineSpacingStyle lineSpacingStyle,
                     const double lineSpacingFactor,
                     const std::string& text_value,
                     const std::string& style,
                     const double angle, const Layer_CSPtr layer) : CADEntity(layer), geo::Dimension(definition_point, middle_of_text, valign, halign,
                                 lineSpacingStyle, lineSpacingFactor, text_value, style,
                                 angle) {

}

Dimension::Dimension(const geo::Coordinate& definition_point,
                     const geo::Coordinate& middle_of_text,
                     const geo::MText::VAlign valign,
                     const geo::MText::HAlign halign,
                     const geo::MText::LineSpacingStyle lineSpacingStyle,
                     const double lineSpacingFactor,
                     const std::string& text_value,
                     const std::string& style,
                     const double angle, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer), geo::Dimension(definition_point, middle_of_text, valign, halign,
                                 lineSpacingStyle, lineSpacingFactor, text_value, style,
                                 angle) {

}


CADEntity_CSPtr Dimension::move(const geo::Coordinate& offset) const {
    auto newDimension = std::make_shared<Dimension>(this->definition_point() + offset, this->middle_of_text() + offset,
                                                    this->valign(), this->halign(),
                                                    this->lineSpacingStyle(), this->lineSpacingFactor(),
                                                    this->text_value(), this->style(), this->angle(), layer());
    newDimension->setID(this->id());
    return newDimension;
}

CADEntity_CSPtr Dimension::copy(const geo::Coordinate& offset) const {
    auto newDimension = std::make_shared<Dimension>(this->definition_point() + offset, this->middle_of_text() + offset,
                                                    this->valign(), this->halign(),
                                                    this->lineSpacingStyle(), this->lineSpacingFactor(),
                                                    this->text_value(), this->style(), this->angle(), layer());
    return newDimension;
}

CADEntity_CSPtr Dimension::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newDimension = std::make_shared<Dimension>(this->definition_point().rotate(rotation_center, rotation_angle), this->middle_of_text(),
                                                    this->valign(), this->halign(),
                                                    this->lineSpacingStyle(), this->lineSpacingFactor(),
                                                    this->text_value(), this->style(), this->angle(), layer());
    newDimension->setID(this->id());
    return newDimension;
}

CADEntity_CSPtr Dimension::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newDimension = std::make_shared<Dimension>(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
                                                    this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                                    this->text_value(), this->style(), this->angle(), layer());
    newDimension->setID(this->id());
    return newDimension;
}
