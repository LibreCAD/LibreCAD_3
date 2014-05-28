#include "builderops.h"
#include "cad/document/document.h"

#include "cad/document/storagemanager.h"


using namespace lc;
using namespace lc::operation;




Begin::Begin() :  Base() {
}

std::vector<CADEntity_CSPtr> Begin::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>& workingBuffer,
    std::vector<CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr>) {
    _entities.insert(_entities.end(), entitySet.begin(), entitySet.end());
    return entitySet;
}

std::vector<CADEntity_CSPtr> Begin::getEntities() const {
    return _entities;
}



Loop::Loop(const int numTimes) :  Base(), _numTimes(numTimes) {
}

std::vector<CADEntity_CSPtr> Loop::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>& _workingBuffer,
    std::vector<CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> _stack) {
    std::vector<CADEntity_CSPtr> final;

    // Find the start
    std::vector<CADEntity_CSPtr> _start;

    for (auto base : _stack) {
        const lc::operation::Begin* begin = dynamic_cast<const lc::operation::Begin*>(base.get());

        if (begin != nullptr) {
            auto entities = begin->getEntities();
            _start.insert(_start.end(), entities.begin(), entities.end());
        }
    }


    // run the operation que
    std::vector<CADEntity_CSPtr> entitySet2(entitySet);

    for (int n = 0; n < _numTimes - 1; n++) {
        for (auto base : _stack) {
            entitySet2 = base->process(storageManager, entitySet2, _workingBuffer, removals, _stack);
        }
    }

    return entitySet2;
}



Move::Move(const geo::Coordinate& offset) :  Base(), _offset(offset) {
}

std::vector<CADEntity_CSPtr>  Move::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>&,
    std::vector<CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<CADEntity_CSPtr> newQueue;

    for (auto entity : entitySet) {
        auto e = entity->move(_offset);
        newQueue.push_back(e);
    }

    return newQueue;
}





Copy::Copy(const geo::Coordinate& offset) : Base(), _offset(offset) {
}

std::vector<CADEntity_CSPtr> Copy::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>& workingBuffer,
    std::vector<CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<CADEntity_CSPtr> newQueue;

    for (auto entity : entitySet) {
        auto e = entity->copy(_offset);
        workingBuffer.push_back(entity);
        newQueue.push_back(e);
    }

    return newQueue;
}

Scale::Scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) : Base(), _scale_center(scale_center), _scale_factor(scale_factor) {
}

std::vector<CADEntity_CSPtr> Scale::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>&,
    std::vector<CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<CADEntity_CSPtr> newQueue;

    for (auto entity : entitySet) {
        auto e = entity->scale(_scale_center, _scale_factor);
        newQueue.push_back(e);
    }

    return newQueue;
}

Rotate::Rotate(const geo::Coordinate& rotation_center, const double rotation_angle) : Base(), _rotation_center(rotation_center), _rotation_angle(rotation_angle) {
}

std::vector<CADEntity_CSPtr> Rotate::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>&,
    std::vector<CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<CADEntity_CSPtr> newQueue;

    for (auto entity : entitySet) {
        auto e = entity->rotate(_rotation_center, _rotation_angle);
        newQueue.push_back(e);
    }

    return newQueue;
}


Push::Push() : Base() {
}

std::vector<CADEntity_CSPtr> Push::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>& workingBuffer,
    std::vector<CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<CADEntity_CSPtr> newQueue(workingBuffer);
    newQueue.insert(newQueue.end(), entitySet.begin(), entitySet.end());
    workingBuffer.clear();
    return newQueue;
}




SelectByLayer::SelectByLayer(const Layer_CSPtr layer) : Base(), _layer(layer) {
}

std::vector<CADEntity_CSPtr> SelectByLayer::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>&,
    std::vector<CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    auto c = storageManager->entitiesByLayer(_layer);

    return c.allEntities(SHRT_MAX);
}




Remove::Remove() : Base() {
}

std::vector<CADEntity_CSPtr> Remove::process(
    const StorageManager_SPtr storageManager,
    const std::vector<CADEntity_CSPtr> entitySet,
    std::vector<CADEntity_CSPtr>&,
    std::vector<CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr>) {
    removals.insert(removals.end(), entitySet.begin(), entitySet.end());
    std::vector<CADEntity_CSPtr> e;
    return e;
}
