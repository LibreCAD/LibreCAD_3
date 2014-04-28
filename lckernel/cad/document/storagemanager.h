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
            /*!
             * \brief layer
             * Return a single document layer
             * \param layerName
             * \return
             */
            virtual shared_ptr<const Layer> layer(const QString& layerName) const = 0;

            /*!
             * \brief Returns all the layers present in the document.
             * \return Hash Layername, Layer
             */
            virtual QHash <QString, shared_ptr<const Layer> > const& allLayers() const = 0;

            virtual shared_ptr<const CADEntity> findEntityByID(ID_DATATYPE id) const = 0;
            virtual EntityContainer findEntitiesByLayer(const shared_ptr<const Layer> layer) const = 0;
    };
}
#endif // ENTITYMANAGER_H
