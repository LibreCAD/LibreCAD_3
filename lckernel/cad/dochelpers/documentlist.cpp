#include "documentlist.h"

using namespace lc;

DocumentList::DocumentList() {

}

void DocumentList::addDocument(lc::Document* document) {
    _documents.insert(document);
    document->newWaitingCustomEntityEvent().connect<DocumentList, &DocumentList::onNewWaitingCustomEntity>(this);
}

void DocumentList::removeDocument(lc::Document* document) {
    document->newWaitingCustomEntityEvent().disconnect<DocumentList, &DocumentList::onNewWaitingCustomEntity>(this);
    _documents.erase(document);
}

void DocumentList::onNewWaitingCustomEntity(const NewWaitingCustomEntityEvent& event) {
    _newWaitingCustomEntityEvent(event);
}

Nano::Signal<void(const NewWaitingCustomEntityEvent&)>& DocumentList::newWaitingCustomEntityEvent() {
    return _newWaitingCustomEntityEvent;
}

std::unordered_set<entity::Insert_CSPtr> DocumentList::waitingCustomEntities(const std::string& pluginName) {
    std::unordered_set<entity::Insert_CSPtr> result;

    for(auto document : _documents) {
        auto entities = document->waitingCustomEntities(pluginName);
        result.insert(entities.begin(), entities.end());
    }

    return result;
}
