#include "entityops.h"
#include "cad/storage/document.h"

#include "cad/storage/storagemanager.h"
using namespace lc;
using namespace lc::operation;

/********************************************************************************************************/
/** Base                                                                                              ***/
/********************************************************************************************************/
Begin::Begin() :  Base() {
}

std::vector<entity::CADEntity_CSPtr> Begin::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    _entities.insert(_entities.end(), entities.begin(), entities.end());
    return entities;
}

std::vector<entity::CADEntity_CSPtr> Begin::getEntities() const {
    return _entities;
}

/********************************************************************************************************/
/** Loop                                                                                              ***/
/********************************************************************************************************/
Loop::Loop(const int numTimes) :  Base(), _numTimes(numTimes) {

}

std::vector<entity::CADEntity_CSPtr> Loop::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    std::vector<entity::CADEntity_CSPtr> final;

    // Find the start
    std::vector<entity::CADEntity_CSPtr> _start;

    for (const auto& base : operationStack) {
        //TODO can we get rid of dynamic_cast??
        auto begin = dynamic_cast<const lc::operation::Begin*>(base.get());

        if (begin != nullptr) {
            auto entities = begin->getEntities();
            _start.insert(_start.end(), entities.begin(), entities.end());
        }
    }


    // run the operation queue
    std::vector<entity::CADEntity_CSPtr> entitySet2(entities);

    for (int n = 0; n < _numTimes - 1; n++) {
        for (const auto& base : operationStack) {
            entitySet2 = base->process(document, entitySet2, workingBuffer, removals, operationStack);
        }
    }

    return entitySet2;
}

/********************************************************************************************************/
/** Move                                                                                              ***/
/********************************************************************************************************/
Move::Move(geo::Coordinate offset) :
        Base(),
        _offset(std::move(offset)) {
}

std::vector<entity::CADEntity_CSPtr>  Move::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entities) {
        auto e = entity->move(_offset);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Copy                                                                                              ***/
/********************************************************************************************************/
Copy::Copy(geo::Coordinate offset) :
        Base(),
        _offset(std::move(offset)) {
}

std::vector<entity::CADEntity_CSPtr> Copy::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entities) {
        auto e = entity->copy(_offset);
        workingBuffer.push_back(entity);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Scale                                                                                             ***/
/********************************************************************************************************/
Scale::Scale(geo::Coordinate scale_center, geo::Coordinate scale_factor) :
        Base(),
        _scale_center(std::move(scale_center)),
        _scale_factor(std::move(scale_factor)) {
}

std::vector<entity::CADEntity_CSPtr> Scale::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entities) {
        auto e = entity->scale(_scale_center, _scale_factor);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Rotate                                                                                            ***/
/********************************************************************************************************/
Rotate::Rotate(geo::Coordinate rotation_center, const double rotation_angle) :
        Base(),
        _rotation_center(std::move(rotation_center)),
        _rotation_angle(rotation_angle) {
}

std::vector<entity::CADEntity_CSPtr> Rotate::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entities) {
        auto e = entity->rotate(_rotation_center, _rotation_angle);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Push                                                                                              ***/
/********************************************************************************************************/
Push::Push() : Base() {
}

std::vector<entity::CADEntity_CSPtr> Push::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    std::vector<entity::CADEntity_CSPtr> newQueue(workingBuffer);
    newQueue.insert(newQueue.end(), entities.begin(), entities.end());
    workingBuffer.clear();
    return newQueue;
}

/********************************************************************************************************/
/** SelectByLayer                                                                                     ***/
/********************************************************************************************************/
SelectByLayer::SelectByLayer(meta::Layer_CSPtr layer) :
        Base(),
        _layer(std::move(layer)) {
}

std::vector<entity::CADEntity_CSPtr> SelectByLayer::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {

    std::vector<entity::CADEntity_CSPtr> e;

    for(auto it = workingBuffer.begin(); it != workingBuffer.end();) {
        if((*it)->layer() == _layer) {
            e.push_back(*it);
            it = workingBuffer.erase(it);
        }
        else {
            it++;
        }
    }

    return e;
}

/********************************************************************************************************/
/** Remove                                                                                            ***/
/********************************************************************************************************/
Remove::Remove() : Base() {

}

std::vector<entity::CADEntity_CSPtr> Remove::process(
    const std::shared_ptr<storage::Document> document,
    const std::vector<entity::CADEntity_CSPtr> entities,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    removals.insert(removals.end(), entities.begin(), entities.end());
    std::vector<entity::CADEntity_CSPtr> e;
    return e;
}