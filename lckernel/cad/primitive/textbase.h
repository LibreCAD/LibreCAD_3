#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"


#include "cad/base/cadentity.h"
#include "cad/math/lcmath.h"
#include "cad/interface/draggable.h"
#include <cad/builders/textbase.h>

namespace lc {
    namespace entity {
        class TextBase {

        public:
            /**
            * @brief TextBase, default constructor
            * @param geo::Coordinate insertion_point, insertion_point of the text
            * @param string text_value, the text itself
            * @param double height, height of the text
            * @param double rotation, angle of rotation
            * @param string style, name of text style
            * @param Layer_CSPtr layer, Layer of the entity
            * @param metatypes, metatypes of the cad entity
            * @param Block_CSPtr, block of the cad entity
            */
            TextBase(geo::Coordinate insertion_point,
                std::string text_value,
                double height,
                double angle,
                std::string style
            );

            TextBase(const TextBase& other);

        protected:
            TextBase(const builder::TextBaseBuilder& builder);

        public:
            const geo::Coordinate _insertion_point;
            const std::string _text_value;
            const double _height;
            const double _angle;
            const std::string _style;

            geo::Coordinate insertion_point() const {
                return _insertion_point;
            }

            std::string const& text_value() const {
                return _text_value;
            }

            double const angle() const {
                return _angle;
            }

            std::string const& style() const {
                return _style;
            }

            double height() const {
                return _height;
            }

        public:
            void getTextProperties(PropertiesMap& propertyValues) const;

            void setTextProperties(const PropertiesMap& propertiesMap, geo::Coordinate& insertionPointp, std::string& textValuep,
                double& heightp, double& anglep) const;
        };

        DECLARE_SHORT_SHARED_PTR(TextBase)
    }
}

