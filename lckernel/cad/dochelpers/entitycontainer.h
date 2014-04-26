#include <cad/base/cadentity.h>

#ifndef ENTITYCONTAINER_H
#define ENTITYCONTAINER_H


namespace lc {
    /**
     * @brief The EntityContainer class
     * manages a set of entities. You can 'spatialy' (not yet optmised) entities within the container.
     *
     */
    class EntityContainer {
        public:
            EntityContainer();

            /*!
             * \brief add an entity to the document.
             * \param entity entity to be added to the document.
             */
            virtual void addEntity(shared_ptr<const CADEntity> entity);
            /*!
             * \brief remove an Entity from the document.
             * \param id Entity ID of entity which is to be removed.
             */
            virtual void removeEntity(shared_ptr<const CADEntity> entity);
            /*!
             * \brief replace an Entity from the document.
             * \param id Entity ID of entity which is to be removed.
             */
            virtual void replaceEntity(shared_ptr<const CADEntity> toReplaceWith);
            /*!
             * \brief return all the entities present in the document.
             * \return QHash Entity ID and Entity.
             */
            virtual QHash<int, shared_ptr<const CADEntity> > allEntities() const;

            virtual shared_ptr<const CADEntity> findEntityByID(ID_DATATYPE id) const;
        private:
            QHash<int, shared_ptr<const CADEntity> > _cadentities; /*!< QHash CADEntity ID -> CADEntity */

    };
}

#endif // ENTITYCONTAINER_H
