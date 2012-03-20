#ifndef PEN_H
#define PEN_H


#include <QMetaType>
#include <QVariant>

#include "color.h"
#include "pen.h"
#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"

namespace lc {
    class Color;
    class LineWidth;

    class Pen : public Variantable, MetaType {
        public:

            Pen();
            Pen(Color* color, LineWidth* lineWidth);
            ~Pen();

            LineWidth* lineWidth() const;
            Color* color() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }

            bool variantValid() {
                return _color != NULL && _lineWidth != NULL;
            }

            int metaName() {
                return CONST::PEN;
            }

        private:

            Color* _color;
            LineWidth* _lineWidth;
    };
}

Q_DECLARE_METATYPE(lc::Pen)



#endif // PEN_H
