#pragma once

#include "cadentity.h"
#include <cad/primitive/textconst.h>

namespace lc {
    namespace builder {
        class TextBuilder : public CADEntityBuilder
        {
        public:
            TextBuilder();

            const lc::geo::Coordinate& insertionPoint() const;

            TextBuilder* setInsertionPoint(const lc::geo::Coordinate& coord);

            const std::string& textValue() const;

            TextBuilder* setTextValue(const std::string& text);

            double height() const;

            TextBuilder* setHeight(double heightIn);

            double angle() const;

            TextBuilder* setAngle(double angleIn);

            entity::Text_CSPtr build();

            TextBuilder* setDrawingDirection(lc::TextConst::DrawingDirection drawingDirectionIn);

            lc::TextConst::DrawingDirection drawingDirection() const;

            TextBuilder* setHorizontalAlign(lc::TextConst::HAlign halignin);

            lc::TextConst::HAlign horizontalAlign() const;

            TextBuilder* setVerticalAlign(lc::TextConst::VAlign valignin);

            lc::TextConst::VAlign verticalAlign() const;

            TextBuilder* setTextFont(std::string font);

            std::string textStyle() const;

            void copy(lc::entity::Text_CSPtr entity);

        private:
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
