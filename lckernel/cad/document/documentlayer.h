#ifndef DOCUMENTLAYER_H
#define DOCUMENTLAYER_H

#include "cad/const.h"

#include "cad/meta/layer.h"
#include "cad/base/cadentity.h"
#include "cad/interface/variantable.h"

namespace lc {

    /**
     * Abstract class for a DocumentLayer
     *
     * it provides a interface to interact with the entities on the layer
     *
     */
    class DocumentLayer : public Variantable {

        public:
            /**
             * /brief Add a entity to this layer
             * @param entity to be added.
             */
            virtual void addEntity(shared_ptr<const CADEntity> entity) = 0;

            /**
             * /brief Remove a entity from this layer
             * @param id ID of the entity to be removed.
             */
            virtual void removeEntity(ID_DATATYPE id) = 0;

            /**
             * /brief Returns Layer.
             * Return a pointer to the layer meta object
             * @return Layer
             */
            virtual shared_ptr<const Layer> layer() const = 0;

            /**
             * /brief Returns ID, Entity Hash
             * Return a list of all entities
             * @return QHash ID, Entity.
             */
            virtual QHash<int, shared_ptr<const CADEntity> > allEntities() const = 0;

            /**
             * /brief Finds an entity by its ID.
             * Return pointer to entity by ID
             * @return CADEntity
             */
            virtual shared_ptr<const CADEntity> findByID(ID_DATATYPE) const = 0;
    };
}


#endif // DOCUMENTLAYER_H
