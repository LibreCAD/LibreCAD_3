#include <cad/dochelpers/documentlist.h>
#include "document.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

Document::Document() {
    DocumentList::getInstance().addDocument(this);
}

Document::~Document() {
    DocumentList::getInstance().removeDocument(this);
}

void Document::operationStart(operation::DocumentOperation_SPtr operation) {
    operation->start();
}
void Document::operationFinnish(operation::DocumentOperation_SPtr operation) {
    operation->finnish();
}
void Document::operationProcess(operation::DocumentOperation_SPtr operation) {
    operation->process();
}
Nano::Signal<void(const lc::BeginProcessEvent&)>& Document::beginProcessEvent() {
    return _beginProcessEvent;
}

Nano::Signal<void(const lc::CommitProcessEvent&)>& Document::commitProcessEvent() {
    return _commitProcessEvent;
}

Nano::Signal<void(const lc::AddEntityEvent&)>& Document::addEntityEvent() {
    return this->_addEntityEvent;
}

Nano::Signal<void(const lc::ReplaceEntityEvent&)>& Document::replaceEntityEvent() {
    return this->_replaceEntityEvent;
}

Nano::Signal<void(const lc::RemoveEntityEvent&)>& Document::removeEntityEvent() {
    return this->_removeEntityEvent;
}

Nano::Signal<void(const lc::RemoveLayerEvent&)>& Document::removeLayerEvent() {
    return this->_removeLayerEvent;
}

Nano::Signal<void(const lc::AddLayerEvent&)>& Document::addLayerEvent() {
    return this->_addLayerEvent;
}

Nano::Signal<void(const lc::ReplaceLayerEvent&)>& Document::replaceLayerEvent() {
    return this->_replaceLayerEvent;
}

Nano::Signal<void(const lc::RemoveLinePatternEvent&)>& Document::removeLinePatternEvent() {
    return this->_removeLinePatternEvent;
}

Nano::Signal<void(const lc::AddLinePatternEvent&)>& Document::addLinePatternEvent() {
    return this->_addLinePatternEvent;
}

Nano::Signal<void(const lc::ReplaceLinePatternEvent&)>& Document::replaceLinePatternEvent() {
    return this->_replaceLinePatternEvent;
}

Nano::Signal<void(const NewWaitingCustomEntityEvent&)>& Document::newWaitingCustomEntityEvent() {
    return _newWaitingCustomEntityEvent;
}
