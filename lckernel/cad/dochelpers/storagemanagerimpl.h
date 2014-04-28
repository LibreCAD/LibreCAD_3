#ifndef ENTITYMANAGERIMPL_H
#define ENTITYMANAGERIMPL_H

#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/storagemanager.h"
#include "cad/events/addentityevent.h"
#include "cad/events/addlayerevent.h"
#include "cad/events/removelayerevent.h"
#include "cad/events/replacelayerevent.h"
#include "cad/events/removeentityevent.h"
#include "entitycontainer.h"

#include <cad/events/replaceentityevent.h>

namespace lc {
    /**
     * A default storag emanager for document's.
     * The data will be cerated in memory and will be lost when the application terminates
     *
     * @param document
     */
    class StorageManagerImpl : public StorageManager {
            Q_OBJECT
        public:
            /*!
                 * \brief Entity manager implementor.
                 * \param document
                 */
            StorageManagerImpl(Document* document);

        public slots:
            /*!
             * \brief Slot for on_addEntityEvent
             * \sa AddEntityEvent
             */
            void on_addLayerEvent(const lc::AddLayerEvent&);

            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa RemoveEntityEvent
             */
            void on_removeLayerEvent(const lc::RemoveLayerEvent&);

            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa RemoveEntityEvent
             */
            void on_replaceLayerEvent(const lc::ReplaceLayerEvent&);

            /*!
             * \brief Slot for on_addEntityEvent
             * \sa AddEntityEvent
             */
            void on_addEntityEvent(const lc::AddEntityEvent&);

            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa RemoveEntityEvent
             */
            void on_removeEntityEvent(const lc::RemoveEntityEvent&);

            /*!
             * \brief Slot for on_replaceEntityEvent
             * \sa ReplaceEntityEvent
             */
            void on_replaceEntityEvent(const ReplaceEntityEvent& event);

        public:
            virtual shared_ptr<const Layer> layer(const QString& layerName) const;
            virtual QHash <QString, shared_ptr<const Layer> > const& allLayers() const;
            virtual shared_ptr<const CADEntity> findEntityByID(ID_DATATYPE id) const;
            virtual EntityContainer findEntitiesByLayer(const shared_ptr<const Layer> layer) const;

        private:
            Document* _document;
            EntityContainer _entities;
            QHash <QString, shared_ptr<const Layer> > _layers;
    };
}
#endif // ENTITYMANAGERIMPL_H
