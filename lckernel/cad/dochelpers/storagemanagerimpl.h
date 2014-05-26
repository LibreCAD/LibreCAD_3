#ifndef ENTITYMANAGERIMPL_H
#define ENTITYMANAGERIMPL_H

#include "cad/const.h"
#include "cad/base/cadentity.h"
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
        public:
            /*!
             * \brief Entity manager implementor.
             * \param document
             */
            StorageManagerImpl();

        public:
            virtual void insertEntity(const CADEntity_CSPtr);
            virtual void insertEntityContainer(const EntityContainer<CADEntity_CSPtr>&);
            virtual void removeEntity(const CADEntity_CSPtr);
            virtual CADEntity_CSPtr entityByID(const ID_DATATYPE id) const;
            virtual EntityContainer<CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) const;

            virtual Layer_CSPtr layerByName(const std::string& layerName) const;
            virtual std::map<std::string, Layer_CSPtr> allLayers() const;

            virtual EntityContainer<CADEntity_CSPtr> entityContainer() const;
            virtual void optimise();
        private:
            EntityContainer<CADEntity_CSPtr> _entities;
            std::map<std::string, Layer_CSPtr> _layers;
    };
}
#endif // ENTITYMANAGERIMPL_H
