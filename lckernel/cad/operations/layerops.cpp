#include "layerops.h"
#include "cad/document/document.h"

using namespace lc;
using namespace operation;

/********************************************************************************************************/
/** AddLayer                                                                                          ***/
/********************************************************************************************************/
AddLayer::AddLayer(std::shared_ptr<Document> document, Layer_CSPtr layer) :
        DocumentOperation(std::move(document), "AddLayer"),
        _layer(std::move(layer)) {
}

void AddLayer::processInternal() {
    document()->addDocumentMetaType(_layer);
}

void AddLayer::undo() const {
    document()->removeDocumentMetaType(_layer);
}

void AddLayer::redo() const {
    document()->addDocumentMetaType(_layer);
}

/********************************************************************************************************/
/** RemoveLayer                                                                                       ***/
/********************************************************************************************************/
RemoveLayer::RemoveLayer(std::shared_ptr<Document> document, Layer_CSPtr layer) :
        DocumentOperation(std::move(document), "RemoveLayer"),
        _layer(std::move(layer)) {

    if(_layer->name() == "0") {
        throw "Layer 0 cannot be removed";
    }

}

void RemoveLayer::processInternal() {
    auto le = document()->entityContainer().entitiesByLayer(_layer).asVector();
    _entities.insert(_entities.end(), le.begin(), le.end());

    for (const auto& i : _entities) {
        document()->removeEntity(i);
    }

    document()->removeDocumentMetaType(_layer);
}

void RemoveLayer::undo() const {
    document()->addDocumentMetaType(_layer);

    for (const auto& i : _entities) {
        document()->insertEntity(i);
    }
}

void RemoveLayer::redo() const {
    for (const auto& i : _entities) {
        document()->removeEntity(i);
    }

    document()->removeDocumentMetaType(_layer);
}


/********************************************************************************************************/
/** ReplaceLayer                                                                                       ***/
/********************************************************************************************************/
ReplaceLayer::ReplaceLayer(std::shared_ptr<Document> document, Layer_CSPtr oldLayer, Layer_CSPtr newLayer) :
        DocumentOperation(std::move(document), "ReplaceLayer"),
        _oldLayer(std::move(oldLayer)),
        _newLayer(std::move(newLayer)) {
}

void ReplaceLayer::processInternal() {
    redo();
}

void ReplaceLayer::undo() const {
    auto le = document()->entityContainer().entitiesByLayer(_newLayer).asVector();

    for (const auto& i : le) {
        document()->removeEntity(i);
        document()->insertEntity(i->modify(_oldLayer, i->metaInfo(), i->block()));
    }

    document()->removeDocumentMetaType(_newLayer);
    document()->addDocumentMetaType(_oldLayer);
}

void ReplaceLayer::redo() const {
    auto le = document()->entityContainer().entitiesByLayer(_oldLayer).asVector();

    for (const auto& i : le) {
        document()->removeEntity(i);
        document()->insertEntity(i->modify(_newLayer, i->metaInfo(), i->block()));
    }

    document()->removeDocumentMetaType(_oldLayer);
    document()->addDocumentMetaType(_newLayer);
}
