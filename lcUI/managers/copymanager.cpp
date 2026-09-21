#include "copymanager.h"

#include <memory>
#include <set>
#include <string>

#include <cad/operations/builder.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/entityops.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/insert.h>

using namespace lc::ui;

namespace {
struct Clipboard {
    std::vector<lc::entity::CADEntity_CSPtr> entities;
    /// The drawing the entities were copied from.  Weak, so the clipboard
    /// does not decide how long a closed drawing lives.
    std::weak_ptr<lc::storage::Document> source;
};

Clipboard& clipboard() {
    static Clipboard instance;
    return instance;
}
}

CopyManager::CopyManager(CadMdiChild* cadMdiChild)
    :
    _cadMdiChild(cadMdiChild)
{}

void CopyManager::copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities) {
    if (cadEntities.empty()) {
        return;
    }

    clipboard().entities = cadEntities;
    clipboard().source = _cadMdiChild->document();
}

void CopyManager::cutEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities) {
    if (cadEntities.empty()) {
        return;
    }

    copyEntitiesToClipboard(cadEntities);

    auto entityBuilder = std::make_shared<lc::operation::EntityBuilder>(_cadMdiChild->document());
    for (const auto& entity : cadEntities) {
        entityBuilder->appendEntity(entity);
    }
    entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
    entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
    entityBuilder->execute();
}

std::vector<lc::entity::CADEntity_CSPtr> CopyManager::pasteableEntities() const {
    const bool sameDrawing = clipboard().source.lock() == _cadMdiChild->document();

    std::vector<lc::entity::CADEntity_CSPtr> entities;
    for (const auto& entity : clipboard().entities) {
        if (sameDrawing || std::dynamic_pointer_cast<const lc::entity::Insert>(entity) == nullptr) {
            entities.push_back(entity);
        }
    }
    return entities;
}

lc::geo::Coordinate CopyManager::basePoint() const {
    const auto entities = pasteableEntities();
    if (entities.empty()) {
        return lc::geo::Coordinate();
    }

    lc::geo::Area extent = entities.front()->boundingBox();
    for (const auto& entity : entities) {
        extent = extent.merge(entity->boundingBox());
    }
    return extent.minP();
}

std::size_t CopyManager::paste(const lc::geo::Coordinate& offset) {
    const auto entities = pasteableEntities();
    if (entities.empty()) {
        return 0;
    }

    auto document = _cadMdiChild->document();
    auto builder = std::make_shared<lc::operation::Builder>(document, "Paste");
    auto entityBuilder = std::make_shared<lc::operation::EntityBuilder>(document);
    std::set<std::string> addedLayers;

    for (const auto& entity : entities) {
        lc::meta::Layer_CSPtr layer = entity->layer();
        if (layer == nullptr) {
            layer = _cadMdiChild->activeLayer();
        }
        else if (auto sameName = document->layerByName(layer->name())) {
            layer = sameName;
        }
        else if (addedLayers.insert(layer->name()).second) {
            builder->append(std::make_shared<lc::operation::AddLayer>(document, layer));
        }

        entityBuilder->appendEntity(
            entity->copy(offset)->modify(layer, entity->metaInfo(), _cadMdiChild->activeViewport()));
    }

    builder->append(entityBuilder);
    builder->execute();
    return entities.size();
}
