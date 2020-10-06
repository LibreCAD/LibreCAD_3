#include "text.h"


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
    _insertion_point(std::move(insertion_point)),
    _text_value(std::move(text_value)),
    _height(height),
    _angle(angle),
    _style(std::move(style)),
    _textgeneration(textgeneration),
    _valign(valign),
    _halign(halign),
    _underlined(underlined),
    _strikethrough(strikethrough),
    _bold(bold),
    _italic(italic)
{
}

Text::Text(const builder::TextBuilder& builder)
    :
    CADEntity(builder),
    _insertion_point(builder.insertionPoint()),
    _text_value(builder.textValue()),
    _height(builder.height()),
    _angle(builder.angle()),
    _style(builder.textStyle()),
    _textgeneration(builder.drawingDirection()),
    _halign(builder.horizontalAlign()),
    _valign(builder.verticalAlign()),
    _underlined(builder.underlined()),
    _strikethrough(builder.strikethrough()),
    _bold(builder.bold()),
    _italic(builder.italic())
{
}

Text::Text(const Text_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    _insertion_point(other->_insertion_point),
    _text_value(other->_text_value),
    _height(other->_height),
    _angle(other->_angle),
    _style(other->_style),
    _textgeneration(other->_textgeneration),
    _valign(other->_valign),
    _halign(other->_halign),
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

    propertyValues["insertionPoint"] = this->insertion_point();
    propertyValues["textValue"] = this->text_value();
    propertyValues["height"] = this->height();
    propertyValues["angle"] = AngleProperty(this->angle());
    propertyValues["bold"] = this->bold();
    propertyValues["italic"] = this->italic();
    propertyValues["underlined"] = this->underlined();
    propertyValues["strikethrough"] = this->strikethrough();

    return propertyValues;
}

CADEntity_CSPtr Text::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate insertionPointp = this->insertion_point();
    std::string textValuep = this->text_value();
    double heightp = this->height();
    double anglep = this->angle();
    bool boldp = this->bold();
    bool italicp = this->italic();
    bool underlinedp = this->underlined();
    bool strikethroughp = this->strikethrough();

    for(auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
        if (iter->first == "insertionPoint") {
            insertionPointp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "textValue") {
            textValuep = boost::get<std::string>(iter->second);
        }

        if (iter->first == "height") {
            heightp = boost::get<double>(iter->second);
        }

        if (iter->first == "angle") {
            anglep = boost::get<AngleProperty>(iter->second).Get();
        }

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
