#pragma once

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include "cad/meta/color.h"

#include <cmath>

namespace lc {

    /**
    * Represents a MetaColor
    */
    class MetaColor : public MetaType {
        public:

            MetaColor() : _color(1., 1., 1., 1.), MetaType() {
            }

            MetaColor(const MetaColor& other) : _color(other.color()), MetaType() {
            }

            MetaColor(const Color& other) : _color(other), MetaType() {
            }

            //MetaColor(int r, int g, int b, int a=0xff) : Color(r, g, b, a), MetaType() {
            //}

            MetaColor(double r, double g, double b, double a = 1.) : _color(r, g, b, a), MetaType() {
            }

            virtual ~MetaColor() {
            }

            virtual std::string lcMetaName() const {
                return MetaColor::LCMETANAME();
            }
            static std::string LCMETANAME() {
                return "_COLOR";
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

            friend std::ostream& operator<<(std::ostream& os, const MetaColor& metaColor) {
                os << "MetaColor(red=" << metaColor.red() << " green=" << metaColor.green() << " blue=" << metaColor.blue() << " alpha" << metaColor.alpha() << ")";
                return os;
            }
    };

    typedef std::shared_ptr<MetaColor> MetaColor_SPtr;
    typedef std::shared_ptr<const MetaColor> MetaColor_CSPtr;
}

