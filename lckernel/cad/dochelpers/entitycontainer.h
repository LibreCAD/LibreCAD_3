
#ifndef ENTITYCONTAINER_H
#define ENTITYCONTAINER_H

#include <QHash>
#include "cad/base/id.h"

namespace lc {
    class Layer;
    class CADEntity;
    /**
     * @brief The EntityContainer class
     * manages a set of entities. You can 'spatialy' (not yet optmised) entities within the container.
     *
     * Note: we should consider this class extending QHash
     */
    class EntityContainer {
        public:
            EntityContainer();

            /*!
             * \brief add an entity to the EntityContainer
             * If the entity already exists, it will be replaced
             * \param entity entity to be added to the document.
             */
            virtual void insert(std::shared_ptr<const CADEntity> entity);

            /*!
             * \brief Add all entities to this container
             * Any entity that already exists will get replaced
             * \param EntityContainer to be combined to the document.
             */
            virtual void combine(const EntityContainer& entities);

            /*!
             * \brief remove an Entity from the document.
             * \param id Entity ID of entity which is to be removed.
             */
            virtual void remove(std::shared_ptr<const CADEntity> entity);

            /*!
             * \brief return all the entities present in the document.
             * \return QHash Entity ID and Entity.
             */
            virtual QHash<ID_DATATYPE, std::shared_ptr<const CADEntity> > allEntities() const;


            virtual std::shared_ptr<const CADEntity> entityByID(ID_DATATYPE id) const;

            /*!
             * \brief findEntitiesByLayer
             * Return a entities container withh all entities from a sepcific layer
             * \param layer
             * \return
             */
            virtual EntityContainer entitiesByLayer(const std::shared_ptr<const Layer> layer) const;
        private:
            QHash<ID_DATATYPE, std::shared_ptr<const CADEntity> > _cadentities;

    };
}

#endif // ENTITYCONTAINER_H
