#pragma once

#include "cad/base/id.h"
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"
#include "cad/dochelpers/entitycontainer.h"

#include <map>
namespace lc {
    /**
     * Entity manager is responsible for adding/removing entities from a document
     */
    class Layer;
    class StorageManager {
        public:
            virtual void insertEntity(const entity::CADEntity_CSPtr) = 0;
            virtual void insertEntityContainer(const EntityContainer<entity::CADEntity_CSPtr>&) = 0;
            virtual void removeEntity(const entity::CADEntity_CSPtr) = 0;
            virtual entity::CADEntity_CSPtr entityByID(ID_DATATYPE id) const = 0;
            virtual EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) const = 0;

            /*!
             * \brief layer
             * Return a single document layer
             * \param layerName
             * \return
             */
            virtual Layer_CSPtr layerByName(const std::string& layerName) const = 0;

            /*!
             * \brief Returns all the layers present in the document.
             * \return Hash Layername, Layer
             */
            virtual std::map<std::string, Layer_CSPtr> allLayers() const = 0;

            /*!
             * \brief entityContainer
             * return a copy of all entities managed within the storage manager
             * \return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entityContainer() const = 0;

            /**
            *  \brief add a new layer to the document
            *  \param layer layer to be added.
            */
            virtual void addLayer(const Layer_CSPtr layer) = 0;

            /**
            *  \brief remove a layer from the document
            *  \param layer layer to be added.
            */
            virtual void removeLayer(const Layer_CSPtr layer) = 0;

            /**
            *  \brief remove a layer from the document
            */
            virtual void replaceLayer(const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer) = 0;

            /**
             * @brief optimise
             * the underlaying data store. Run this at a regular base, for example after each task
             */
            virtual void optimise() = 0;
    };

    class StorageManager;
    using StorageManager_SPtr = std::shared_ptr<StorageManager>;
    using StorageManager_CSPtr = std::shared_ptr<const StorageManager>;
}
