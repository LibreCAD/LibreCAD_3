#include "undomanagerimpl.h"

#include <QDebug>

#include "cad/operations/documentoperation.h"
#include "cad/operations/undoable.h"

using namespace lc;


UndoManagerImpl::UndoManagerImpl(Document* document, int maximumUndoLevels) : _document(document), _maximumUndoLevels(maximumUndoLevels) {

    connect(document, SIGNAL(commitProcessEvent(const lc::CommitProcessEvent&)),
            this, SLOT(on_CommitProcessEvent(const lc::CommitProcessEvent&)));
}


void UndoManagerImpl::on_CommitProcessEvent(const CommitProcessEvent& event) {

    shared_ptr<operation::Undoable>  undoable = dynamic_pointer_cast<operation::Undoable>(event.operation());

    if (undoable != NULL) {
        qDebug() << "Process: " << undoable->text();

        // Check if Redo is possible, if so we might need to purge objects from memory
        // as long as we can redo, purge these objects
        while (canRedo()) {
            shared_ptr<operation::Undoable>  undoable = _reDoables.pop();
            // Need to get a list of absolete entities, they are all entities that are created in the _reDoables list
            // document()->absolueteEntity(entity);

        }

        // Add undoable to stack
        _unDoables.append(undoable);

        // Remove old undoables
        if (_unDoables.size() > this->_maximumUndoLevels) {
            undoable = _unDoables.first();
            // Need to get a list of absolete entities, they are all entities that are delete in the _unDoables list
            // document()->absolueteEntity(entity);
            _unDoables.pop_front();
        }
    }
}


void UndoManagerImpl::redo() {
    if (canRedo()) {
        shared_ptr<operation::Undoable>  undoable = _reDoables.pop();
        undoable->redo();
        _unDoables.append(undoable);
    }
}
void UndoManagerImpl::undo() {
    if (canUndo()) {
        shared_ptr<operation::Undoable>  undoable = _unDoables.last();
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

void UndoManagerImpl::removeUndoables() {
    _unDoables.clear();
    _reDoables.clear();
}
