#include "undomanagerimpl.h"

#include <QDebug>

#include "cad/operations/documentoperation.h"
#include "cad/operations/undoable.h"

using namespace lc;


UndoManagerImpl::UndoManagerImpl(Document* document, unsigned int maximumUndoLevels) : _document(document), _maximumUndoLevels(maximumUndoLevels) {

    connect(document, SIGNAL(commitProcessEvent(const lc::CommitProcessEvent&)),
            this, SLOT(on_CommitProcessEvent(const lc::CommitProcessEvent&)));
}


void UndoManagerImpl::on_CommitProcessEvent(const CommitProcessEvent& event) {

    operation::Undoable_SPtr undoable = std::dynamic_pointer_cast<operation::Undoable>(event.operation());

    if (undoable.get() != nullptr) {
        qDebug() << "Process: " << undoable->text();

        // Check if Redo is possible, if so we might need to purge objects from memory
        // as long as we can redo, purge these objects
        while (canRedo()) {
            operation::Undoable_SPtr undoable = _reDoables.top();
            _reDoables.pop();
            // Need to get a list of absolete entities, they are all entities that are created in the _reDoables list
            // document()->absolueteEntity(entity);

        }

        // Add undoable to stack
        _unDoables.push_back(undoable);

        // Remove old undoables
        if (_unDoables.size() > this->_maximumUndoLevels) {
            // Need to get a list of absolete entities, they are all entities that are delete in the _unDoables list
            // document()->absolueteEntity(entity);
            _unDoables.erase(_unDoables.begin(), _unDoables.begin() + 1);
        }
    }
}


void UndoManagerImpl::redo() {
    if (canRedo()) {
        operation::Undoable_SPtr undoable = _reDoables.top();
        _reDoables.pop();
        undoable->redo();
        _unDoables.push_back(undoable);
    }
}
void UndoManagerImpl::undo() {
    if (canUndo()) {
        operation::Undoable_SPtr undoable = _unDoables.back();
        _unDoables.pop_back();
        undoable->undo();
        _reDoables.push(undoable);
    }
}

bool UndoManagerImpl::canRedo() const {
    return !_reDoables.empty();
}
bool UndoManagerImpl::canUndo() const {
    return !_unDoables.empty();
}

void UndoManagerImpl::removeUndoables() {
    _unDoables.clear();

    while (!_reDoables.empty()) {
        _reDoables.pop();
    }
}
