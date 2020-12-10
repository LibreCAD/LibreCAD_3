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
           bool underlined,
           bool strikethrough,
           bool bold,
           bool italic,
           meta::Layer_CSPtr layer,
           meta::MetaInfo_CSPtr metaInfo,
           meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    TextBase(insertion_point, text_value, height, angle, style, textgeneration, halign, valign),
    _underlined(underlined),
    _strikethrough(strikethrough),
    _bold(bold),
    _italic(italic)
{
}

Text::Text(const builder::TextBuilder& builder)
    :
    CADEntity(builder),
    TextBase(builder),
    _underlined(builder.underlined()),
    _strikethrough(builder.strikethrough()),
    _bold(builder.bold()),
    _italic(builder.italic())
{
}

Text::Text(const Text_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    TextBase(*other),
    _underlined(other->_underlined),
    _strikethrough(other->_strikethrough),
    _bold(other->_bold),
    _italic(other->_italic)
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
                                          this->_underlined,
                                          this->_strikethrough,
                                          this->_bold,
                                          this->_italic,
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
                       this->_underlined,
                       this->_strikethrough,
                       this->_bold,
                       this->_italic,
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
                       this->_underlined,
                       this->_strikethrough,
                       this->_bold,
                       this->_italic,
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
                       this->_underlined,
                       this->_strikethrough,
                       this->_bold,
                       this->_italic,
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
                       this->_underlined,
                       this->_strikethrough,
                       this->_bold,
                       this->_italic,
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
                                                underlined(),
                                                strikethrough(),
                                                bold(),
                                                italic(),
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
    propertyValues["bold"] = this->bold();
    propertyValues["italic"] = this->italic();
    propertyValues["underlined"] = this->underlined();
    propertyValues["strikethrough"] = this->strikethrough();

    return propertyValues;
}

CADEntity_CSPtr Text::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate insertionPointp;
    std::string textValuep;
    double heightp;
    double anglep;
    bool boldp = this->bold();
    bool italicp = this->italic();
    bool underlinedp = this->underlined();
    bool strikethroughp = this->strikethrough();

    setTextProperties(propertiesMap, insertionPointp, textValuep, heightp, anglep);
    for(auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
        if (iter->first == "bold") {
            boldp = boost::get<bool>(iter->second);
        }

        if (iter->first == "italic") {
            italicp = boost::get<bool>(iter->second);
        }

        if (iter->first == "underlined") {
            underlinedp = boost::get<bool>(iter->second);
        }

        if (iter->first == "strikethrough") {
            strikethroughp = boost::get<bool>(iter->second);
        }
    }

    auto textEntity = std::make_shared<Text>(insertionPointp, textValuep, heightp, anglep, style(), textgeneration(), halign(), valign(), underlinedp, strikethroughp, boldp, italicp, layer(), metaInfo(), block());
    textEntity->setID(this->id());
    return textEntity;
}
