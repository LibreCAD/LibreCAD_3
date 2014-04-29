#include "builder.h"
#include "cad/document/document.h"


using namespace lc;
using namespace lc::operation;

Builder::Builder(Document* document) : DocumentOperation(document), Undoable("Builder") {
}

Builder::~Builder() {
    qDebug() << "Builder removed";
}

Builder& Builder::append(shared_ptr<const CADEntity> cadEntity) {
    _workingBuffer.append(cadEntity);
    return *this;
}

Builder& Builder::move(const geo::Coordinate& offset) {
    _stack.append(make_shared<Move>(offset));
    return *this;
}
Builder& Builder::copy(const geo::Coordinate& offset) {
    _stack.append(make_shared<Copy>(offset));
    return *this;
}
Builder& Builder::repeat(const int numTimes) {
    _stack.append(make_shared<Loop>(numTimes));
    return *this;
}
Builder& Builder::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) {
    _stack.append(make_shared<Rotate>(rotation_center, rotation_angle));
    return *this;
}
Builder& Builder::begin() {
    _stack.append(make_shared<Begin>());
    return *this;
}
Builder& Builder::push() {
    _stack.append(make_shared<Push>());
    return *this;
}
Builder& Builder::selectByLayer(const shared_ptr<const Layer> layer) {
    _stack.append(make_shared<SelectByLayer>(layer));
    return *this;
}


void Builder::processInternal(shared_ptr<StorageManager> storageManager) {
    QList<shared_ptr<const CADEntity> > newQueue;

    for (int i = 0; i < _stack.size(); ++i) {
        // Get looping stack, we currently support only one single loop!!
        QList<shared_ptr<Base> > stack = _stack.mid(0, i);
        newQueue = _stack.at(i)->process(storageManager, newQueue, _workingBuffer, stack);
    }

    _workingBuffer.append(newQueue);

    // Build a buffer with all entities we need to remove during a undo cycle
    for (int i = 0; i < _workingBuffer.size(); ++i) {
        auto org = storageManager->entityByID(_workingBuffer.at(i)->id());
        if (org.get()!=NULL) {
            _entitiesToInsert.append(org);
        } else {
            _entitiesToRemove.append(org);
        }
    }

    // Add/Update all entities in the document
    for (int i = 0; i < _workingBuffer.size(); ++i) {
        document()->insertEntity(_workingBuffer.at(i));
    }

    // TODO
    // Remove entities from the document
}

void Builder::undo() const {

    for (int i = 0; i < _entitiesToRemove.size(); ++i) {
        document()->removeEntity(_entitiesToRemove.at(i));
    }

    for (int i = 0; i < _entitiesToInsert.size(); ++i) {
        document()->insertEntity(_entitiesToInsert.at(i));
    }

}

void Builder::redo() const {
    for (int i = 0; i < _entitiesToRemove.size(); ++i) {
        document()->removeEntity(_entitiesToRemove.at(i));
    }

    for (int i = 0; i < _entitiesToInsert.size(); ++i) {
        document()->insertEntity(_entitiesToInsert.at(i));
    }

}

