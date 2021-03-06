#include "text.h"
#include "textbase.h"


using namespace lc;
using namespace entity;

Text::Text(geo::Coordinate insertion_point,
           std::string text_value,
           double height,
           double angle,
           std::string style,
           const TextConst::DrawingDirection textgeneration,
           const TextConst::HAlign halign,
           const TextConst::VAlign valign,
           meta::Layer_CSPtr layer,
           meta::MetaInfo_CSPtr metaInfo,
           meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    TextBase(insertion_point, text_value, height, angle, style, textgeneration, halign, valign)
{
}

Text::Text(const builder::TextBuilder& builder)
    :
    CADEntity(builder),
    TextBase(builder)
{
}

Text::Text(const Text_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    TextBase(*other)
{
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
                                          layer()
                                          , metaInfo(), block()
                                         );
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
                       layer()
                       , metaInfo(), block());
    newText->setID(this->id());
    return newText;
}

CADEntity_CSPtr Text::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    auto newText = std::make_shared<Text>(
                       this->_insertion_point.rotate(rotation_center, rotation_angle),
                       this->_text_value,
                       this->_height,
                       this->_angle,
                       this->_style,
                       this->_textgeneration,
                       this->_halign,
                       this->_valign,
                       layer()
                       , metaInfo(), block());
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
                       this->layer()
                       , metaInfo(), block());
    newText->setID(this->id());
    return newText;
}

const geo::Area Text::boundingBox() const {
    /// @todo Fix this
    // Rough bounding box
    // Assume that the font has char max. width equal to height
    // Assume single line
    double width = this->_height * (this->_text_value).size() / 2;
    return geo::Area(this->_insertion_point - geo::Coordinate(width, width), this->_insertion_point + geo::Coordinate(width, width));
}

CADEntity_CSPtr Text::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newText = std::make_shared<Text>(
                       this->_insertion_point,
                       this->_text_value,
                       this->_height,
                       this->_angle,
                       this->_style,
                       this->_textgeneration,
                       this->_halign,
                       this->_valign,
                       layer,
                       metaInfo,
                       block
                   );
    newText->setID(this->id());

    return newText;
}

std::map<unsigned int, lc::geo::Coordinate> Text::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;

    dragPoints[0] = _insertion_point;

    return dragPoints;
}

CADEntity_CSPtr Text::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<Text>(dragPoints.at(0),
                                                text_value(),
                                                height(),
                                                angle(),
                                                style(),
                                                textgeneration(),
                                                halign(),
                                                valign(),
                                                layer(),
                                                metaInfo(), block()
                                               );
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

PropertiesMap Text::availableProperties() const {
    PropertiesMap propertyValues;

    getTextProperties(propertyValues);
    return propertyValues;
}

CADEntity_CSPtr Text::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate insertionPointp;
    std::string textValuep;
    double heightp;
    double anglep;

    setTextProperties(propertiesMap, insertionPointp, textValuep, heightp, anglep);
    auto textEntity = std::make_shared<Text>(insertionPointp, textValuep, heightp, anglep, style(), textgeneration(), halign(), valign(), layer(), metaInfo(), block());
    textEntity->setID(this->id());
    return textEntity;
}
