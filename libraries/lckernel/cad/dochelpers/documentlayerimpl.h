#ifndef DOCUMENTLAYERIMPL_H
#define DOCUMENTLAYERIMPL_H

#include <QVector>
#include <QObject>

#include "cad/document/documentlayer.h"
#include "cad/meta/layer.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Implementation class of a complete layer and it's entities
     *
     */
    class DocumentLayerImpl : public DocumentLayer {
        public:
            DocumentLayerImpl();
            DocumentLayerImpl(Layer* layer);
            virtual ~DocumentLayerImpl();

            virtual void addEntity(CADEntity* entity);
            virtual void removeEntity(ID_DATATYPE id);
            virtual QVector<CADEntity*>* allEntities() const;

            virtual Layer* layer() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }
            bool variantValid() const {
                return _layer != NULL && _cadentities != NULL;
            }


        private:

        private:
            Layer* _layer;
            QVector<CADEntity*>* _cadentities;
    };
}

Q_DECLARE_METATYPE(lc::DocumentLayerImpl)
#endif // DOCUMENTLAYERIMPL_H
