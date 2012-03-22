#ifndef DOCUMENTLAYERIMPL_H
#define DOCUMENTLAYERIMPL_H

#include <QVector>

#include "documentlayer.h"
#include "cad/meta/layer.h"
#include "cad/base/cadentity.h"

namespace lc {
    class DocumentLayerImpl : public DocumentLayer {
        public:
            DocumentLayerImpl();
            DocumentLayerImpl(Layer* layer);
            virtual ~DocumentLayerImpl();

            virtual void addEntity(CADEntity* entity);

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }
            bool variantValid() const {
                return _layer != NULL && _cadentities != NULL;
            }

        private:
            Layer* _layer;
            QVector<CADEntity*>* _cadentities;
    };
}

Q_DECLARE_METATYPE(lc::DocumentLayerImpl)
#endif // DOCUMENTLAYERIMPL_H
