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

#include "cad/events/replaceentityevent.h"
#include "cad/meta/dxflinepattern.h"
#include "cad/functions/string_helper.h"

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
             * \param entity::CADEntity_CSPtr
             */
            virtual void insertEntity(const entity::CADEntity_CSPtr);

            /**
             * @brief remove Entity from the container
             * \param entity::CADEntity_CSPtr
             */
            virtual void removeEntity(const entity::CADEntity_CSPtr);

            /**
             * @brief insertEntityContainer
             * \param EntityContainer<entity::CADEntity_CSPtr>
             */
            virtual void insertEntityContainer(const EntityContainer<entity::CADEntity_CSPtr> &);

           /**
             * @brief return entity By ID
             * @param id
             * @return entity::CADEntity_CSPtr entity
             */
            virtual entity::CADEntity_CSPtr entityByID(const ID_DATATYPE id) const;

            /**
             * @brief Returns entities By Layer
             * @param layer
             * @return EntityContainer<entity::CADEntity_CSPtr> entities on layer
             * @Deprecated use entityContainer()->entitiesByLayer()
             */
            virtual lc::EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) const;

            /**
             * @brief returns layer By Name
             * @param layerName
             * @return Layer_CSPtr layer
             */
            virtual Layer_CSPtr layerByName(const std::string& layerName) const;

            /**
             * @brief returns line pattern By Name
             * @param string& linePatternName
             * @return DxfLinePattern_CSPtr layer
             */
            virtual DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const;

            /**
             * @brief return all Layers
             * @return map<string, Layer_CSPtr>
             */
            virtual std::map<std::string, Layer_CSPtr> allLayers() const;

            /**
             * @brief returns entity Container
             * @return entityContainer<entity::CADEntity_CSPtr>
             */
            virtual lc::EntityContainer<entity::CADEntity_CSPtr> entityContainer() const;

            /**
            *  \brief add a document meta type
            *  \param layer layer to be added.
            */
            virtual void addDocumentMetaType(const DocumentMetaType_CSPtr dmt);

            /**
            *  \brief remove a document meta type from the document
            *  \param layer layer to be added.
            */
            virtual void removeDocumentMetaType(const DocumentMetaType_CSPtr dmt);

            /**
            *  \brief remove document meta type
            */
            virtual void replaceDocumentMetaType(const DocumentMetaType_CSPtr oldDmt, const DocumentMetaType_CSPtr newDmt);

            virtual std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive> allMetaTypes() const;

            lc::EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(const Block_CSPtr block) const override;

            /**
             * @brief optimise the quadtree
             */
            virtual void optimise();


    private:
            virtual DocumentMetaType_CSPtr _metaDataTypeByName(const std::string id) const;

    private:

            EntityContainer<entity::CADEntity_CSPtr> _entities;
            std::map<std::string, DocumentMetaType_CSPtr, StringHelper::cmpCaseInsensetive> _documentMetaData;
            std::map<std::string, lc::EntityContainer<entity::CADEntity_CSPtr>> _blocksEntities;
    };
}
