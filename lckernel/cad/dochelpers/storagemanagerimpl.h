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
#include <map>
#include <utility>
#include <string>

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

            virtual ~StorageManagerImpl();

            /**
             * @brief insertEntity
             * \param entity::CADEntity_CSPtr
             */
            void insertEntity(entity::CADEntity_CSPtr) override;

            /**
             * @brief remove Entity from the container
             * \param entity::CADEntity_CSPtr
             */
            void removeEntity(entity::CADEntity_CSPtr) override;

            /**
             * @brief insertEntityContainer
             * \param EntityContainer<entity::CADEntity_CSPtr>
             */
            void insertEntityContainer(const EntityContainer<entity::CADEntity_CSPtr>&) override;

            /**
              * @brief return entity By ID
              * @param id
              * @return entity::CADEntity_CSPtr entity
              */
            entity::CADEntity_CSPtr entityByID(ID_DATATYPE id) const override;

            /**
             * @brief Returns entities By Layer
             * @param layer
             * @return EntityContainer<entity::CADEntity_CSPtr> entities on layer
             * @Deprecated use entityContainer()->entitiesByLayer()
             */
            lc::EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) const override;

            /**
             * @brief returns layer By Name
             * @param layerName
             * @return Layer_CSPtr layer
             */
            Layer_CSPtr layerByName(const std::string& layerName) const override;

            /**
             * @brief returns line pattern By Name
             * @param string& linePatternName
             * @return DxfLinePattern_CSPtr layer
             */
            DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;

            /**
             * @brief return all Layers
             * @return map<string, Layer_CSPtr>
             */
            std::map<std::string, Layer_CSPtr> allLayers() const override;

            /**
             * @brief returns entity Container
             * @return entityContainer<entity::CADEntity_CSPtr>
             */
            lc::EntityContainer<entity::CADEntity_CSPtr> entityContainer() const override;

            /**
            *  \brief add a document meta type
            *  \param layer layer to be added.
            */
            void addDocumentMetaType(DocumentMetaType_CSPtr dmt) override;

            /**
            *  \brief remove a document meta type from the document
            *  \param layer layer to be added.
            */
            void removeDocumentMetaType(DocumentMetaType_CSPtr dmt) override;

            /**
            *  \brief remove document meta type
            */
            void replaceDocumentMetaType(DocumentMetaType_CSPtr oldDmt, DocumentMetaType_CSPtr newDmt) override;

            std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive>
            allMetaTypes() const override;

            lc::EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(Block_CSPtr block) const override;

            /**
             * @brief optimise the quadtree
             */
            void optimise() override;

    private:
            DocumentMetaType_CSPtr _metaDataTypeByName(const std::string& id) const override;

            EntityContainer<entity::CADEntity_CSPtr> _entities;
            std::map<std::string, DocumentMetaType_CSPtr, StringHelper::cmpCaseInsensetive> _documentMetaData;
            std::map<std::string, lc::EntityContainer<entity::CADEntity_CSPtr>> _blocksEntities;
    };
}
