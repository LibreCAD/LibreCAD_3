#include "layerops.h"
#include "cad/document/document.h"

using namespace lc;
using namespace lc::operation;

/********************************************************************************************************/
/** AddLayer                                                                                          ***/
/********************************************************************************************************/
AddLayer::AddLayer(Document* document,  const Layer_SPtr layer) : DocumentOperation(document), Undoable("AddLayer"), _layer(layer)  {
}

void AddLayer::processInternal() {
    document()->addLayer(_layer);
}

void AddLayer::undo() const {
    document()->removeLayer(_layer);
}

void AddLayer::redo() const {
    document()->addLayer(_layer);
}

/********************************************************************************************************/
/** RemoveLayer                                                                                       ***/
/********************************************************************************************************/
RemoveLayer::RemoveLayer(Document* document,  const Layer_SPtr layer) : DocumentOperation(document), Undoable("RemoveLayer"), _layer(layer)  {
}

void RemoveLayer::processInternal() {
    auto le = document()->entityContainer().entitiesByLayer(_layer).asVector();
    _entities.insert(_entities.end(), le.begin(), le.end());
    for (auto i : _entities) {
        document()->removeEntity(i);
    }
    document()->removeLayer(_layer);
}

void RemoveLayer::undo() const {
    document()->addLayer(_layer);
    for (auto i : _entities) {
        document()->insertEntity(i);
    }
}

void RemoveLayer::redo() const {
    for (auto i : _entities) {
        document()->removeEntity(i);
    }
    document()->removeLayer(_layer);
}


/********************************************************************************************************/
/** ReplaceLayer                                                                                       ***/
/********************************************************************************************************/
ReplaceLayer::ReplaceLayer(Document* document,  const Layer_SPtr oldLayer,  const Layer_SPtr newLayer) : DocumentOperation(document), Undoable("ReplaceLayer"), _oldLayer(oldLayer), _newLayer(newLayer)  {
}

void ReplaceLayer::processInternal() {
    auto le = document()->entityContainer().entitiesByLayer(_oldLayer).asVector();
    for (auto i : le) {
//        document()->removeEntity(i);
    }
    document()->removeLayer(_oldLayer);
    document()->removeLayer(_newLayer);
    for (auto i : le) {
//        document()->removeEntity(i);
    }
}

void ReplaceLayer::undo() const {
}

void ReplaceLayer::redo() const {
}
