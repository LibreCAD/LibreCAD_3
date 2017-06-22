#pragma once

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include "cad/meta/color.h"

#include <cmath>

namespace lc {
    /**
    * Represents a MetaColor
    */
    class MetaColor : public EntityMetaType {
        protected:
            MetaColor() : EntityMetaType() {

            }

        public:
            virtual const std::string metaTypeID() const override {
                return LCMETANAME();
            }
            static const std::string LCMETANAME() {
                return "_COLOR";
            }
    };

    class MetaColorByValue : public MetaColor {
        public:

            MetaColorByValue() : MetaColor(), _color(1., 1., 1., 1.) {
            }

            MetaColorByValue(const MetaColorByValue& other) : MetaColor(), _color(other.color()) {
            }

            MetaColorByValue(const Color& other) : MetaColor(), _color(other) {
            }

            MetaColorByValue(double r, double g, double b, double a = 1.) : MetaColor(), _color(r, g, b, a) {
            }

            virtual ~MetaColorByValue() = default;

            virtual const std::string id() const override {
                // TODO create proper ID
                return LCMETANAME() + "_" + std::to_string(red()) + "_" +  std::to_string(green()) + "_" +  std::to_string(blue());
            }

            Color color() const {
                return _color;
            }

            inline double red() const {
                return _color.red();
            }

            inline double green() const {
                return _color.green();
            }

            inline double blue() const {
                return _color.blue();
            }

            inline double alpha() const {
                return _color.alpha();
            }

            inline unsigned char redI() const {
                return _color.redI();
            }

            inline unsigned char greenI() const {
                return _color.greenI();
            }

            inline unsigned char blueI() const {
                return _color.blueI();
            }

            inline unsigned char alphaI() const {
                return _color.alphaI();
            }

        private:
            Color _color;

            friend std::ostream& operator<<(std::ostream& os, const MetaColorByValue& metaColor) {
                os << "MetaColorByValue(red=" << metaColor.red() << " green=" << metaColor.green() << " blue=" << metaColor.blue() << " alpha" << metaColor.alpha() << ")";
                return os;
            }
    };

    class MetaColorByBlock : public MetaColor {
        public:
            MetaColorByBlock() {

            }

            virtual const std::string id() const override {
                return LCMETANAME() + "_BYBLOCK";
            }
    };

    using MetaColor_SPtr = std::shared_ptr<MetaColor>;
    using MetaColor_CSPtr = std::shared_ptr<const MetaColor>;
    using MetaColorByValue_SPtr = std::shared_ptr<MetaColorByValue>;
    using MetaColorByValue_CSPtr = std::shared_ptr<const MetaColorByValue>;
    using MetaColorByBlock_SPtr = std::shared_ptr<MetaColorByBlock>;
    using MetaColorByBlock_CSPtr = std::shared_ptr<const MetaColorByBlock>;
}

