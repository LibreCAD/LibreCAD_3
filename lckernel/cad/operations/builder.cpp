#include "builder.h"
#include "cad/document/document.h"
#include <memory>


using namespace lc;
using namespace lc::operation;

Builder::Builder(Document* document) : DocumentOperation(document), Undoable("Builder") {
}

Builder::~Builder() {
    qDebug() << "Builder removed";
}

Builder& Builder::append(CADEntity_CSPtr cadEntity) {
    _workingBuffer.append(cadEntity);
    return *this;
}

Builder& Builder::move(const geo::Coordinate& offset) {
    _stack.append(std::make_shared<Move>(offset));
    return *this;
}
Builder& Builder::copy(const geo::Coordinate& offset) {
    _stack.append(std::make_shared<Copy>(offset));
    return *this;
}
Builder& Builder::repeat(const int numTimes) {
    _stack.append(std::make_shared<Loop>(numTimes));
    return *this;
}
Builder& Builder::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) {
    _stack.append(std::make_shared<Rotate>(rotation_center, rotation_angle));
    return *this;
}
Builder& Builder::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) {
    _stack.append(std::make_shared<Scale>(scale_center, scale_factor));
    return *this;
}

Builder& Builder::begin() {
    _stack.append(std::make_shared<Begin>());
    return *this;
}
Builder& Builder::push() {
    _stack.append(std::make_shared<Push>());
    return *this;
}
Builder& Builder::selectByLayer(const Layer_CSPtr layer) {
    _stack.append(std::make_shared<SelectByLayer>(layer));
    return *this;
}
Builder& Builder::remove() {
    _stack.append(std::make_shared<Remove>());
    return *this;
}


void Builder::processInternal(StorageManager_SPtr storageManager) {
    QList<CADEntity_CSPtr> entitySet;

    for (int i = 0; i < _stack.size(); ++i) {
        // Get looping stack, we currently support only one single loop!!
        QList<Base_SPtr> stack = _stack.mid(0, i);
        entitySet = _stack.at(i)->process(storageManager, entitySet, _workingBuffer, _entitiesThatNeedsRemoval, stack);
    }

    _workingBuffer.append(entitySet);

    // Build a buffer with all entities we need to remove during a undo cycle
    for (int i = 0; i < _workingBuffer.size(); ++i) {
        auto org = storageManager->entityByID(_workingBuffer.at(i)->id());

        if (org.get() != nullptr) {
            _entitiesThatWhereUpdated.append(org);
        }
    }

    // Remove entities
    for (int i = 0; i < _entitiesThatNeedsRemoval.size(); ++i) {
        document()->removeEntity(_entitiesThatNeedsRemoval.at(i));
    }

    // Add/Update all entities in the document
    for (int i = 0; i < _workingBuffer.size(); ++i) {
        document()->insertEntity(_workingBuffer.at(i));
    }

}

void Builder::undo() const {

    for (int i = 0; i < _workingBuffer.size(); ++i) {
        document()->removeEntity(_workingBuffer.at(i));
    }

    for (int i = 0; i < _entitiesThatWhereUpdated.size(); ++i) {
        document()->insertEntity(_entitiesThatWhereUpdated.at(i));
    }

    for (int i = 0; i < _entitiesThatNeedsRemoval.size(); ++i) {
        document()->insertEntity(_entitiesThatNeedsRemoval.at(i));
    }

}

void Builder::redo() const {
    for (int i = 0; i < _entitiesThatNeedsRemoval.size(); ++i) {
        document()->removeEntity(_entitiesThatNeedsRemoval.at(i));
    }

    for (int i = 0; i < _workingBuffer.size(); ++i) {
        document()->insertEntity(_workingBuffer.at(i));
    }

}

