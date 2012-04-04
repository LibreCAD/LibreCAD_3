#ifndef DOCUMENTLAYERIMPL_H
#define DOCUMENTLAYERIMPL_H

#include <QVector>
#include <QObject>

#include "cad/const.h"

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

            virtual void addEntity(CADEntityPtr entity);
            virtual void removeEntity(ID_DATATYPE id);
            virtual QHash<int, CADEntityPtr>  const& allEntities() const;

            virtual Layer* layer() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }
            bool variantValid() const {
                return _layer != NULL;
            }

            virtual CADEntityPtr findByID(ID_DATATYPE) const;

        private:

        private:
            Layer* _layer;
            QHash<int, CADEntityPtr> _cadentities;
    };
}

Q_DECLARE_METATYPE(lc::DocumentLayerImpl)
#endif // DOCUMENTLAYERIMPL_H
