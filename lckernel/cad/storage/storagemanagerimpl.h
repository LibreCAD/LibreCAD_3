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
 * The data will be created in memory and will be lost when the application terminates
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

    void insertEntity(entity::CADEntity_CSPtr) override;
    void removeEntity(entity::CADEntity_CSPtr) override;
    void insertEntityContainer(const EntityContainer <entity::CADEntity_CSPtr>&) override;
    entity::CADEntity_CSPtr entityByID(ID_DATATYPE id) const override;
    EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const meta::Layer_CSPtr layer) const override;
    meta::Layer_CSPtr layerByName(const std::string& layerName) const override;
    meta::Block_CSPtr blockByName(const std::string& blockName) const override;
    meta::DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const override;
    std::map<std::string, meta::Layer_CSPtr> allLayers() const override;
    EntityContainer <entity::CADEntity_CSPtr>& entityContainer() override;
    void addDocumentMetaType(meta::DocumentMetaType_CSPtr dmt) override;
    void removeDocumentMetaType(meta::DocumentMetaType_CSPtr dmt) override;
    void replaceDocumentMetaType(meta::DocumentMetaType_CSPtr oldDmt, meta::DocumentMetaType_CSPtr newDmt) override;
    std::map<std::string, meta::DocumentMetaType_CSPtr, lc::tools::StringHelper::cmpCaseInsensitive> allMetaTypes() const override;
    EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(meta::Block_CSPtr block) const override;
    void optimise() override;

private:
    meta::DocumentMetaType_CSPtr _metaDataTypeByName(const std::string& id) const override;

    EntityContainer <entity::CADEntity_CSPtr> _entities;
    std::map<std::string, meta::DocumentMetaType_CSPtr, tools::StringHelper::cmpCaseInsensitive> _documentMetaData;
    std::map<std::string, EntityContainer<entity::CADEntity_CSPtr> > _blocksEntities;
};
}
}
