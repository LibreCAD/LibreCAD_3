#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <QObject>

#include "cad/const.h"
#include "cad/base/id.h"
#include "cad/meta/layer.h"
#include "cad/dochelpers/entitycontainer.h"
namespace lc {
    /**
     * Entity manager is responsible for adding/removing entities from a document
     */
    class CADEntity;
    class StorageManager : public QObject {
            Q_OBJECT
        public:
            virtual void insertEntity(const std::shared_ptr<const CADEntity>) = 0;
            virtual void insertEntityContainer(const EntityContainer&) = 0;
            virtual void removeEntity(const std::shared_ptr<const CADEntity>) = 0;
            virtual std::shared_ptr<const CADEntity> entityByID(ID_DATATYPE id) const = 0;
            virtual EntityContainer entitiesByLayer(const std::shared_ptr<const Layer> layer) const = 0;

            /*!
             * \brief layer
             * Return a single document layer
             * \param layerName
             * \return
             */
            virtual std::shared_ptr<const Layer> layerByName(const QString& layerName) const = 0;

            /*!
             * \brief Returns all the layers present in the document.
             * \return Hash Layername, Layer
             */
            virtual QHash <QString, std::shared_ptr<const Layer> > const& allLayers() const = 0;

            virtual EntityContainer entityContainer() const = 0;
    };
}
#endif // ENTITYMANAGER_H
