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
            DocumentLayerImpl(std::tr1::shared_ptr<const lc::Layer> layer);
            virtual ~DocumentLayerImpl();

            virtual void addEntity(std::tr1::shared_ptr<const lc::CADEntity> entity);
            virtual void removeEntity(ID_DATATYPE id);
            virtual QHash<int, std::tr1::shared_ptr<const lc::CADEntity> > allEntities() const;

            virtual std::tr1::shared_ptr<const lc::Layer> layer() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }
            bool variantValid() const {
                return _layer != NULL;
            }

            virtual std::tr1::shared_ptr<const lc::CADEntity> findByID(ID_DATATYPE) const;

        private:

        private:
            std::tr1::shared_ptr<const lc::Layer> _layer;
            QHash<int, std::tr1::shared_ptr<const lc::CADEntity> > _cadentities;
    };
}

Q_DECLARE_METATYPE(lc::DocumentLayerImpl)
#endif // DOCUMENTLAYERIMPL_H
