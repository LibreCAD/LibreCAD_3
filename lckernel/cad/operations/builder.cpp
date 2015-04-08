#include "builder.h"
#include "cad/document/document.h"
#include <memory>


using namespace lc;
using namespace lc::operation;

Builder::Builder(std::shared_ptr<Document> document) : DocumentOperation(document), Undoable("Builder") {
}

Builder::~Builder() {
    // LOG4CXX_DEBUG(logger, "Builder removed");
}

Builder& Builder::append(entity::CADEntity_CSPtr cadEntity) {
    _workingBuffer.push_back(cadEntity);
    return *this;
}

Builder& Builder::move(const geo::Coordinate& offset) {
    _stack.push_back(std::make_shared<Move>(offset));
    return *this;
}
Builder& Builder::copy(const geo::Coordinate& offset) {
    _stack.push_back(std::make_shared<Copy>(offset));
    return *this;
}
Builder& Builder::repeat(const int numTimes) {
    _stack.push_back(std::make_shared<Loop>(numTimes));
    return *this;
}
Builder& Builder::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) {
    _stack.push_back(std::make_shared<Rotate>(rotation_center, rotation_angle));
    return *this;
}
Builder& Builder::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) {
    _stack.push_back(std::make_shared<Scale>(scale_center, scale_factor));
    return *this;
}

Builder& Builder::begin() {
    _stack.push_back(std::make_shared<Begin>());
    return *this;
}
Builder& Builder::push() {
    _stack.push_back(std::make_shared<Push>());
    return *this;
}
Builder& Builder::selectByLayer(const Layer_CSPtr layer) {
    _stack.push_back(std::make_shared<SelectByLayer>(layer));
    return *this;
}
Builder& Builder::remove() {
    _stack.push_back(std::make_shared<Remove>());
    return *this;
}


void Builder::processInternal() {
    std::vector<entity::CADEntity_CSPtr> entitySet;

    for (auto it = _stack.begin(); it != _stack.end(); ++it) {
        // Get looping stack, we currently support only one single loop!!
        std::vector<Base_SPtr> stack(_stack.begin(), it);
        entitySet = (*it)->process(document(), entitySet, _workingBuffer, _entitiesThatNeedsRemoval, stack);
    }

    _workingBuffer.insert(_workingBuffer.end(), entitySet.begin(), entitySet.end());

    auto ec = document()->entityContainer();

    // Build a buffer with all entities we need to remove during a undo cycle
    for (auto entity : _workingBuffer) {
        // Todo, consider changing this, we only need to know if this entity already exists
        auto org = ec.entityByID(entity->id());

        if (org.get() != nullptr) {
            _entitiesThatWhereUpdated.push_back(org);
        }
    }

    // Remove entities
    for (auto entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    // Add/Update all entities in the document
    for (auto entity : _workingBuffer) {
        document()->insertEntity(entity);
    }
}

void Builder::undo() const {
    for (auto entity : _workingBuffer) {
        document()->removeEntity(entity);
    }

    for (auto entity : _entitiesThatWhereUpdated) {
        document()->insertEntity(entity);
    }

    for (auto entity : _entitiesThatNeedsRemoval) {
        document()->insertEntity(entity);
    }
}

void Builder::redo() const {
    for (auto entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    for (auto entity : _workingBuffer) {
        document()->insertEntity(entity);
    }
}

