#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <QObject>

#include "cad/base/id.h"
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"
namespace lc {
    /**
     * Entity manager is responsible for adding/removing entities from a document
     */
    class EntityContainer;
    class Layer;
    class StorageManager : public QObject {
            Q_OBJECT
        public:
            virtual void insertEntity(const CADEntity_CSPtr) = 0;
            virtual void insertEntityContainer(const EntityContainer&) = 0;
            virtual void removeEntity(const CADEntity_CSPtr) = 0;
            virtual CADEntity_CSPtr entityByID(ID_DATATYPE id) const = 0;
            virtual EntityContainer entitiesByLayer(const Layer_CSPtr layer) const = 0;

            /*!
             * \brief layer
             * Return a single document layer
             * \param layerName
             * \return
             */
            virtual Layer_CSPtr layerByName(const QString& layerName) const = 0;

            /*!
             * \brief Returns all the layers present in the document.
             * \return Hash Layername, Layer
             */
            virtual std::map<QString, Layer_CSPtr> allLayers() const = 0;

            /*!
             * \brief entityContainer
             * return a copy of all entities managed within the storage manager
             * \return
             */
            virtual EntityContainer entityContainer() const = 0;
    };

    class StorageManager;
    typedef std::shared_ptr<StorageManager> StorageManager_SPtr;
    typedef std::shared_ptr<const StorageManager> StorageManager_CSPtr;
}
#endif // ENTITYMANAGER_H
