#pragma once

#include <cad/storage/document.h>
#include <unordered_set>
#include <string>

using namespace lc;

namespace lc {
namespace storage {
class DocumentList {
    friend class Document;

public:
    // One registry owned by lckernel, not a header-local copy in each DLL.
    static DocumentList& getInstance();

    DocumentList(DocumentList const&) = delete;

    void operator=(DocumentList const&) = delete;

    /**
     * @brief Event called when an unmanaged entity is added to a document
     */
    Nano::Signal<void(const event::NewWaitingCustomEntityEvent&)>& newWaitingCustomEntityEvent();

    std::unordered_set<entity::Insert_CSPtr> waitingCustomEntities(const std::string& pluginName);

private:
    DocumentList() = default;

    void addDocument(Document* document);

    void removeDocument(Document* document);

    void onNewWaitingCustomEntity(const event::NewWaitingCustomEntityEvent& event);

    std::unordered_set<Document*> _documents;
    Nano::Signal<void(const lc::event::NewWaitingCustomEntityEvent&)> _newWaitingCustomEntityEvent;
};
}
}
