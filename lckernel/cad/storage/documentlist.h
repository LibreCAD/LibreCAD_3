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
                static DocumentList& getInstance() {
                    static DocumentList _instance;

                    return _instance;
                }

                DocumentList(DocumentList const&) = delete;

                void operator=(DocumentList const&) = delete;

                /**
                 * @brief Event called when an unmanaged entity is added to a document
                 */
                Nano::Signal<void(const NewWaitingCustomEntityEvent&)>& newWaitingCustomEntityEvent();

                std::unordered_set<entity::Insert_CSPtr> waitingCustomEntities(const std::string& pluginName);

            private:
                DocumentList() = default;

                void addDocument(Document* document);

                void removeDocument(Document* document);

                void onNewWaitingCustomEntity(const NewWaitingCustomEntityEvent& event);

                std::unordered_set<Document*> _documents;
                Nano::Signal<void(const lc::NewWaitingCustomEntityEvent&)> _newWaitingCustomEntityEvent;
        };
    }
}
