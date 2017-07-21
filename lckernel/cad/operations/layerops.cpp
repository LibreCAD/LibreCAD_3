#include "layerops.h"
#include "cad/document/document.h"

using namespace lc;
using namespace operation;

/********************************************************************************************************/
/** AddLayer                                                                                          ***/
/********************************************************************************************************/
AddLayer::AddLayer(std::shared_ptr<Document> document, const Layer_CSPtr layer) :
        DocumentOperation(document, "AddLayer"),
        _layer(layer) {
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
RemoveLayer::RemoveLayer(std::shared_ptr<Document> document, const Layer_CSPtr layer) :
        DocumentOperation(document, "RemoveLayer"),
        _layer(layer) {

    if(layer->name() == "0") {
        throw "Layer 0 cannot be removed";
    }

}

void RemoveLayer::processInternal() {
    auto le = document()->entityContainer().entitiesByLayer(_layer).asVector();
    _entities.insert(_entities.end(), le.begin(), le.end());

    for (auto i : _entities) {
        document()->removeEntity(i);
    }

    document()->removeDocumentMetaType(_layer);
}

void RemoveLayer::undo() const {
    document()->addDocumentMetaType(_layer);

    for (auto i : _entities) {
        document()->insertEntity(i);
    }
}

void RemoveLayer::redo() const {
    for (auto i : _entities) {
        document()->removeEntity(i);
    }

    document()->removeDocumentMetaType(_layer);
}


/********************************************************************************************************/
/** ReplaceLayer                                                                                       ***/
/********************************************************************************************************/
ReplaceLayer::ReplaceLayer(std::shared_ptr<Document> document, const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer) :
        DocumentOperation(document, "ReplaceLayer"),
        _oldLayer(oldLayer),
        _newLayer(newLayer) {
}

void ReplaceLayer::processInternal() {
    auto le = document()->entityContainer().entitiesByLayer(_oldLayer).asVector();

    for (auto i : le) {
        document()->removeEntity(i);
        document()->insertEntity(i->modify(_newLayer, i->metaInfo(), i->block()));
    }

    document()->removeDocumentMetaType(_oldLayer);
    document()->addDocumentMetaType(_newLayer);
}

void ReplaceLayer::undo() const {
}

void ReplaceLayer::redo() const {
}
