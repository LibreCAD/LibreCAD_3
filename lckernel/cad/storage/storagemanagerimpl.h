#pragma once

#include "cad/const.h"
#include "cad/base/cadentity.h"
#include "document.h"
#include "storagemanager.h"
#include "cad/events/addentityevent.h"
#include "cad/events/addlayerevent.h"
#include "cad/events/removelayerevent.h"
#include "cad/events/replacelayerevent.h"
#include "cad/events/removeentityevent.h"
#include "entitycontainer.h"

#include "cad/events/replaceentityevent.h"
#include "cad/meta/dxflinepattern.h"
#include "cad/tools/string_helper.h"
#include <map>
#include <utility>
#include <string>

namespace lc {
    namespace storage {
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

                virtual ~StorageManagerImpl() = default;

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
                void insertEntityContainer(const EntityContainer <entity::CADEntity_CSPtr>&) override;

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
                EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const meta::Layer_CSPtr layer) const override;

                /**
                 * @brief returns layer By Name
                 * @param layerName
                 * @return Layer_CSPtr layer
                 */
                meta::Layer_CSPtr layerByName(const std::string& layerName) const override;

                meta::Block_CSPtr blockByName(const std::string& blockName) const override;

                /**
                 * @brief returns line pattern By Name
                 * @param string& linePatternName
                 * @return DxfLinePattern_CSPtr layer
                 */
                meta::DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;

                /**
                 * @brief return all Layers
                 * @return map<string, Layer_CSPtr>
                 */
                std::map<std::string, meta::Layer_CSPtr> allLayers() const override;

                /**
                 * @brief returns entity Container
                 * @return entityContainer<entity::CADEntity_CSPtr>
                 */
                EntityContainer <entity::CADEntity_CSPtr>& entityContainer() override;

                /**
                *  \brief add a document meta type
                *  \param layer layer to be added.
                */
                void addDocumentMetaType(meta::DocumentMetaType_CSPtr dmt) override;

                /**
                *  \brief remove a document meta type from the document
                *  \param layer layer to be added.
                */
                void removeDocumentMetaType(meta::DocumentMetaType_CSPtr dmt) override;

                /**
                *  \brief remove document meta type
                */
                void replaceDocumentMetaType(meta::DocumentMetaType_CSPtr oldDmt, meta::DocumentMetaType_CSPtr newDmt) override;

                std::map<std::string, meta::DocumentMetaType_CSPtr, lc::tools::StringHelper::cmpCaseInsensetive> allMetaTypes() const override;

                EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(meta::Block_CSPtr block) const override;

                /**
                 * @brief optimise the quadtree
                 */
                void optimise() override;

            private:
                meta::DocumentMetaType_CSPtr _metaDataTypeByName(const std::string& id) const override;

                EntityContainer <entity::CADEntity_CSPtr> _entities;
                std::map<std::string, meta::DocumentMetaType_CSPtr, tools::StringHelper::cmpCaseInsensetive> _documentMetaData;
                std::map<std::string, EntityContainer<entity::CADEntity_CSPtr> > _blocksEntities;
        };
    }
}
