#pragma once

#include "cadentity.h"
#include <cad/primitive/textconst.h>

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

            TextBaseBuilder* setDrawingDirection(lc::TextConst::DrawingDirection drawingDirectionIn);

            lc::TextConst::DrawingDirection drawingDirection() const;

            TextBaseBuilder* setHorizontalAlign(lc::TextConst::HAlign halignin);

            lc::TextConst::HAlign horizontalAlign() const;

            TextBaseBuilder* setVerticalAlign(lc::TextConst::VAlign valignin);

            lc::TextConst::VAlign verticalAlign() const;

        protected:
            lc::geo::Coordinate _insertionPoint;
            std::string _textValue;
            std::string _textStyle;
            double _height;
            double _angle;
            lc::TextConst::DrawingDirection _drawingDirection;
            lc::TextConst::HAlign _hAlign;
            lc::TextConst::VAlign _vAlign;
        };
    }
}
