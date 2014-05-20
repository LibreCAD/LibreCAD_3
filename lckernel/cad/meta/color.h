#ifndef COLOR_H
#define COLOR_H

#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"

#include "cad/const.h"


namespace lc {

    /**
      * Represents a Color
      *
      */
    class Color : public MetaType {
        public:
            enum Method {
                Invalid = -1,
                ByLayer = 0,
                ByEntity = 1
            };

            Color();
            /**
              * Values for RGBA are 8 bits
              */
            Color(const int r, const int g, const int b, const int a = 255);
            Color(Method method);
            virtual ~Color() {
            }

            Method method() const;
            int red() const;
            int green() const;
            int blue() const;
            int alpha() const;

            //            QColor qColor() const;

            bool variantValid() const {
                return !(_method == Color::Invalid);
            }

            metaTypeId metaName() const {
                return MetaType::COLOR;
            }

        private:
            Method _method;
            int _r;
            int _g;
            int _b;
            int _a;
    };


}

#endif // Color_H
