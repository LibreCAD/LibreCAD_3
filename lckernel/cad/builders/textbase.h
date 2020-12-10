#pragma once

#include "cadentity.h"

namespace lc {
    namespace builder {
        class TextBaseBuilder : public CADEntityBuilder
        {
        public:
            TextBaseBuilder();

            const lc::geo::Coordinate& insertionPoint() const;

            TextBaseBuilder* setInsertionPoint(const lc::geo::Coordinate& coord);

            const std::string& textValue() const;

            TextBaseBuilder* setTextValue(const std::string& text);

            double height() const;

            TextBaseBuilder* setHeight(double heightIn);

            double angle() const;

            TextBaseBuilder* setAngle(double angleIn);

            TextBaseBuilder* setTextFont(std::string font);

            std::string textStyle() const;

            //void copy(lc::entity::TextBase_CSPtr entity);

        protected:
            lc::geo::Coordinate _insertionPoint;
            std::string _textValue;
            std::string _textStyle;
            double _height;
            double _angle;
        };
    }
}
