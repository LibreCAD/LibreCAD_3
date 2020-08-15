#include <cad/storage/documentlist.h>
#include "document.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc::storage;

Document::Document():
    _beginProcessEvent(),
    _commitProcessEvent(),
    _addEntityEvent(),
    _replaceEntityEvent(),
    _removeEntityEvent(),
    _addLayerEvent(),
    _addViewportEvent(),
    _replaceLayerEvent(),
    _removeLayerEvent(),
    _addLinePatternEvent(),
    _replaceLinePatternEvent(),
    _removeLinePatternEvent(),
    _newWaitingCustomEntityEvent() {
    storage::DocumentList::getInstance().addDocument(this);
}

Document::~Document() {
    storage::DocumentList::getInstance().removeDocument(this);
}

void Document::operationStart(const operation::DocumentOperation_SPtr& operation) {
    operation->start();
}
void Document::operationFinish(const operation::DocumentOperation_SPtr& operation) {
    operation->finnish();
}
void Document::operationProcess(const operation::DocumentOperation_SPtr& operation) {
    operation->process();
}
Nano::Signal<void(const lc::event::BeginProcessEvent&)>& Document::beginProcessEvent() {
    return _beginProcessEvent;
}

Nano::Signal<void(const lc::event::CommitProcessEvent&)>& Document::commitProcessEvent() {
    return _commitProcessEvent;
}

Nano::Signal<void(const lc::event::AddEntityEvent&)>& Document::addEntityEvent() {
    return this->_addEntityEvent;
}

Nano::Signal<void(const lc::event::ReplaceEntityEvent&)>& Document::replaceEntityEvent() {
    return this->_replaceEntityEvent;
}

Nano::Signal<void(const lc::event::RemoveEntityEvent&)>& Document::removeEntityEvent() {
    return this->_removeEntityEvent;
}

Nano::Signal<void(const lc::event::RemoveLayerEvent&)>& Document::removeLayerEvent() {
    return this->_removeLayerEvent;
}

Nano::Signal<void(const lc::event::AddLayerEvent&)>& Document::addLayerEvent() {
    return this->_addLayerEvent;
}

Nano::Signal<void(const lc::event::AddViewportEvent&)>& Document::addViewportEvent() {
    return this->_addViewportEvent;
}

Nano::Signal<void(const lc::event::ReplaceLayerEvent&)>& Document::replaceLayerEvent() {
    return this->_replaceLayerEvent;
}

Nano::Signal<void(const lc::event::RemoveLinePatternEvent&)>& Document::removeLinePatternEvent() {
    return this->_removeLinePatternEvent;
}

Nano::Signal<void(const lc::event::AddLinePatternEvent&)>& Document::addLinePatternEvent() {
    return this->_addLinePatternEvent;
}

Nano::Signal<void(const lc::event::ReplaceLinePatternEvent&)>& Document::replaceLinePatternEvent() {
    return this->_replaceLinePatternEvent;
}

Nano::Signal<void(const lc::event::NewWaitingCustomEntityEvent&)>& Document::newWaitingCustomEntityEvent() {
    return _newWaitingCustomEntityEvent;
}
