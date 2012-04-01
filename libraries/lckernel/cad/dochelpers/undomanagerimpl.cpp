#include "undomanagerimpl.h"

#include <QDebug>

#include "cad/operations/operation.h"
#include "cad/operations/undoable.h"

using namespace lc;


UndoManagerImpl::UndoManagerImpl(int maximumUndoLevels) : UndoManager() {
    _maximumUndoLevels = maximumUndoLevels;
}

void UndoManagerImpl::setDocument(AbstractDocument* document) {
    UndoManager::setDocument(document);
    connect(document, SIGNAL(commitProcessEvent(lc::CommitProcessEvent*)),
            this, SLOT(on_CommitProcessEvent(lc::CommitProcessEvent*)));
}




void UndoManagerImpl::on_CommitProcessEvent(lc::CommitProcessEvent* event) {

    Undoable* undoable = dynamic_cast<Undoable*>(event->operation());

    if (undoable != NULL) {
        qDebug() << "Process: " << undoable->text();

        // Check if Redo is possible, if so we might need to purge objects from memory
        // as long as we can redo, purge these objects
        while (canRedo()) {
            Undoable* undoable = _reDoables.pop();
            // FIXME: remove undoable from memory with associated entities taht can be removed
            // document()->absolueteEntity(entity);

        }

        // Add undoable to stack
        _unDoables.append(undoable);

        if (_unDoables.size() > this->_maximumUndoLevels) {
            undoable = _unDoables.first();
            // FIXME: remove undoable from memory with associated entities that can be removed
            // document()->absolueteEntity(entity);
            _unDoables.pop_front();
        }
    }
}


void UndoManagerImpl::redo() {
    if (canRedo()) {
        Undoable* undoable = _reDoables.pop();
        undoable->redo();
        _unDoables.append(undoable);
    }
}
void UndoManagerImpl::undo() {
    if (canUndo()) {
        Undoable* undoable = _unDoables.last();
        _unDoables.pop_back();
        undoable->undo();
        _reDoables.push(undoable);
    }
}

bool UndoManagerImpl::canRedo() const {
    return !_reDoables.isEmpty();
}
bool UndoManagerImpl::canUndo() const {
    return !_unDoables.isEmpty();
}

void UndoManagerImpl::manageQueue() {

}
