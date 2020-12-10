#include "textbase.h"


using namespace lc;
using namespace entity;

TextBase::TextBase(geo::Coordinate insertion_point,
    std::string text_value,
    double height,
    double angle,
    std::string style) :
    _insertion_point(std::move(insertion_point)),
    _text_value(std::move(text_value)),
    _height(height),
    _angle(angle),
    _style(std::move(style))
{
}

TextBase::TextBase(const builder::TextBaseBuilder& builder)
    :
    _insertion_point(builder.insertionPoint()),
    _text_value(builder.textValue()),
    _height(builder.height()),
    _angle(builder.angle()),
    _style(builder.textStyle())
{
}

TextBase::TextBase(const TextBase& other) :
    _insertion_point(other._insertion_point),
    _text_value(other._text_value),
    _height(other._height),
    _angle(other._angle),
    _style(other._style)
{
}

void TextBase::getTextProperties(PropertiesMap& propertyValues) const {
    propertyValues["insertionPoint"] = this->insertion_point();
    propertyValues["textValue"] = this->text_value();
    propertyValues["height"] = this->height();
    propertyValues["angle"] = AngleProperty(this->angle());
}

void TextBase::setTextProperties(const PropertiesMap& propertiesMap, geo::Coordinate& insertionPointp, std::string& textValuep,
    double& heightp, double& anglep) const {
    insertionPointp = this->insertion_point();
    textValuep = this->text_value();
    heightp = this->height();
    anglep = this->angle();

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
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
    }
}
