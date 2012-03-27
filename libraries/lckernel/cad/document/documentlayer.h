#ifndef DOCUMENTLAYER_H
#define DOCUMENTLAYER_H

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
             * Add a entity to this layer
             * @param entity
             */
            virtual void addEntity(CADEntity* entity) = 0;

            /**
             * Remove a entity from this layer
             * @param id
             */
            virtual void removeEntity(ID_DATATYPE id) = 0;

            /**
             * Return a pointer to the layer meta object
             * @return
             */
            virtual Layer* layer() const = 0;

            /**
             * Return a list of all entities
             *
             * @return
             */
            virtual QVector<CADEntity*>* allEntities() const = 0;
    };
}


#endif // DOCUMENTLAYER_H
