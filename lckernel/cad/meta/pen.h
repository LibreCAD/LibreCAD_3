#ifndef PEN_H
#define PEN_H


#include <QMetaType>
#include <QVariant>

#include "cad/const.h"

#include "color.h"
#include "linewidth.h"
#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"
namespace lc {

    /*!
     * \deprecated Pen is deperecated a user must use color or linewdith
     */
    class Pen : MetaType {
        public:

            Pen();
            Pen(Color* color, LineWidth* lineWidth);
            virtual ~Pen();

            LineWidth* lineWidth() const;
            Color* color() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }

            bool variantValid() const {
                return _color != nullptr && _lineWidth != nullptr;
            }

            int metaName() const {
                return MetaType::PEN;
            }

        private:

            Color* _color;
            LineWidth* _lineWidth;
    };
}

Q_DECLARE_METATYPE(lc::Pen)



#endif // PEN_H
