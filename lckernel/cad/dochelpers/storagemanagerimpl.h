#pragma once

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
     * A default storage manager for document's.
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
            /**
             * @brief insertEntity
             * \param CADEntity_CSPtr
             */
            virtual void insertEntity(const CADEntity_CSPtr);

            /**
             * @brief insertEntityContainer
             * \param EntityContainer<CADEntity_CSPtr>
             */
            virtual void insertEntityContainer(const EntityContainer<CADEntity_CSPtr>&);
            /**
             * @brief remove Entity from the container
             * \param CADEntity_CSPtr
             */
            virtual void removeEntity(const CADEntity_CSPtr);

            /**
             * @brief return entity By ID
             * @param id
             * @return CADEntity_CSPtr entity
             */
            virtual CADEntity_CSPtr entityByID(const ID_DATATYPE id) const;

            /**
             * @brief Returns entities By Layer
             * @param layer
             * @return EntityContainer<CADEntity_CSPtr> entities on layer
             * @Deprecated use entityContainer()->entitiesByLayer()
             */
            virtual EntityContainer<CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) const;

            /**
             * @brief returns layer By Name
             * @param layerName
             * @return Layer_CSPtr layer
             */
            virtual Layer_CSPtr layerByName(const std::string& layerName) const;

            /**
             * @brief return all Layers
             * @return map<string, Layer_CSPtr>
             */
            virtual std::map<std::string, Layer_CSPtr> allLayers() const;

            /**
             * @brief returns entity Container
             * @return entityContainer<CADEntity_CSPtr>
             */
            virtual EntityContainer<CADEntity_CSPtr> entityContainer() const;

            /**
             * @brief optimise the quadtree
             */
            virtual void optimise();
        private:
            EntityContainer<CADEntity_CSPtr> _entities;
            std::map<std::string, Layer_CSPtr> _layers;
    };
}
