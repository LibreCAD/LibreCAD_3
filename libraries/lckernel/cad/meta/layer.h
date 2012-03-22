#ifndef LAYER_H
#define LAYER_H

#include <QMetaType>
#include <QVariant>
#include <QString>

#include "cad/interface/variantable.h"
#include "cad/meta/linewidth.h"
#include "cad/meta/color.h"


namespace lc {
    class Layer : public Variantable {
        public:
            Layer();
            Layer(const QString& name, LineWidth *lineWidth, Color *color);
            virtual ~Layer();

            LineWidth* lineWidth() const;
            Color* color() const;
            QString name() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }

            bool variantValid() const {
                return (_lineWidth != NULL) &&  (_color != NULL);
            }

        private:
            QString _name;
            LineWidth* _lineWidth;
            Color* _color;
    };
}

Q_DECLARE_METATYPE(lc::Layer)

#endif // LAYER_H
