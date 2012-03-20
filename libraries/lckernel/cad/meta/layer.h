#ifndef LAYER_H
#define LAYER_H

#include <QMetaType>
#include <QVariant>
#include <QString>

#include "cad/interface/variantable.h"


namespace lc {
    class Pen;
    class Layer : public Variantable {
        public:
            Layer();
            Layer(const QString& name, Pen* Pen);
            ~Layer();

            Pen* pen() const;
            QString name() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }

            bool variantValid() {
                return !(_pen != NULL);
            }

        private:
            QString _name;
            Pen* _pen;
    };
}

Q_DECLARE_METATYPE(lc::Layer)

#endif // LAYER_H
