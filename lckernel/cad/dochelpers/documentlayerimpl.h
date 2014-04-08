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
            /*!
             * \brief Document layer implementor.
             */
            DocumentLayerImpl();
            /*!
             * \brief Document layer implementor with a layer.
             * \param layer
             */
            DocumentLayerImpl(shared_ptr<const lc::Layer> layer);
            virtual ~DocumentLayerImpl();

            /*!
             * \brief add an entity to the document.
             * \param entity entity to be added to the document.
             */
            virtual void addEntity(shared_ptr<const lc::CADEntity> entity);
            /*!
             * \brief remove an Entity from the document.
             * \param id Entity ID of entity which is to be removed.
             */
            virtual void removeEntity(ID_DATATYPE id);
            /*!
             * \brief return all the entities present in the document.
             * \return QHash Entity ID and Entity.
             */
            virtual QHash<int, shared_ptr<const lc::CADEntity> > allEntities() const;

            virtual shared_ptr<const lc::Layer> layer() const;

            operator QVariant() const {
                return QVariant::fromValue(*this);
            }
            bool variantValid() const {
                return _layer != NULL;
            }

            /*!
             * \brief finds entity by entity ID
             * \return CADEntity shared_ptr
             */
            virtual shared_ptr<const lc::CADEntity> findByID(ID_DATATYPE) const;

        private:

        private:
            shared_ptr<const lc::Layer> _layer; /*!< Layer shared pointer */
            QHash<int, shared_ptr<const lc::CADEntity> > _cadentities; /*!< QHash CADEntity ID -> CADEntity */
    };
}

Q_DECLARE_METATYPE(lc::DocumentLayerImpl)
#endif // DOCUMENTLAYERIMPL_H
