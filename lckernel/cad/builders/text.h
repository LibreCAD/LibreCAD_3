#pragma once

#include "cadentity.h"

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

            entity::Text_CSPtr build();

        private:
            lc::geo::Coordinate _insertionPoint;
            std::string _textValue;
        };
    }
}
