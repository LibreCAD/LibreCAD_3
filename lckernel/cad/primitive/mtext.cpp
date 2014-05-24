#include "cad/primitive/mtext.h"

using namespace lc;

MText::MText(const geo::Coordinate& insertion_point,
             const double height,
             const double width,
             const VAlign valign,
             const HAlign halign,
             const DrawingDirection drawingDirection,
             const LineSpacingStyle lineSpacingStyle,
             const double lineSpacingFactor,
             const std::string& text_value,
             const std::string& style,
             const double angle, const Layer_CSPtr layer) : CADEntity(layer), geo::MText(insertion_point, height, width, valign, halign, drawingDirection,
                         lineSpacingStyle, lineSpacingFactor, text_value, style,
                         angle) {

}

MText::MText(const geo::Coordinate& insertion_point,
             const double height,
             const double width,
             const VAlign valign,
             const HAlign halign,
             const DrawingDirection drawingDirection,
             const LineSpacingStyle lineSpacingStyle,
             const double lineSpacingFactor,
             const std::string& text_value,
             const std::string& style,
             const double angle, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer), geo::MText(insertion_point, height, width, valign, halign, drawingDirection,
                         lineSpacingStyle, lineSpacingFactor, text_value, style,
                         angle) {

}


CADEntity_CSPtr MText::move(const geo::Coordinate& offset) const {
    auto newMText = std::make_shared<MText>(this->insertion_point() + offset,
                                            this->height(), this->width(),
                                            this->valign(), this->halign(),
                                            this->drawingDirection(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                            this->text_value(), this->style(), this->angle(), layer());
    newMText->setID(this->id());
    return newMText;
}

CADEntity_CSPtr MText::copy(const geo::Coordinate& offset) const {
    auto newMText = std::make_shared<MText>(this->insertion_point() + offset,
                                            this->height(), this->width(),
                                            this->valign(), this->halign(),
                                            this->drawingDirection(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                            this->text_value(), this->style(), this->angle(), layer());
    return newMText;
}

CADEntity_CSPtr MText::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newMText = std::make_shared<MText>(this->insertion_point().rotate(rotation_center, rotation_angle),
                                            this->height(), this->width(),
                                            this->valign(), this->halign(),
                                            this->drawingDirection(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                            this->text_value(), this->style(), this->angle(), layer());
    newMText->setID(this->id());
    return newMText;
}

CADEntity_CSPtr MText::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newMText = std::make_shared<MText>(this->insertion_point().scale(scale_center, scale_factor),
                                            this->height() * scale_factor.y(), this->width() * scale_factor.x(),
                                            this->valign(), this->halign(),
                                            this->drawingDirection(), this->lineSpacingStyle(), this->lineSpacingFactor(),
                                            this->text_value(), this->style(), this->angle(), layer());
    newMText->setID(this->id());
    return newMText;
}
