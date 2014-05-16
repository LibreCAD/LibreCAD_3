#ifndef LAYER_H
#define LAYER_H

#include <QMetaType>
#include <QVariant>
#include <QString>

#include "cad/const.h"

#include "cad/interface/variantable.h"
#include "cad/meta/linewidth.h"
#include "cad/meta/color.h"


namespace lc {
    class Layer : public MetaType  {
        public:
            Layer();
            Layer(const QString& name, const LineWidth lineWidth, const Color color);
            virtual ~Layer();

            LineWidth lineWidth() const;
            Color color() const;
            QString name() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }

            bool operator == (const Layer& layer) const {
                return (layer._name == _name);
            }

            bool variantValid() const {
                return true;
            }

            metaTypeId metaName() const {
                return MetaType::LAYER;
            }

        private:
            QString _name;
            LineWidth _lineWidth;
            Color _color;
    };
    typedef std::shared_ptr<Layer> Layer_SPtr;
    typedef std::shared_ptr<const Layer> Layer_CSPtr;
}

Q_DECLARE_METATYPE(lc::Layer)

#endif // LAYER_H
