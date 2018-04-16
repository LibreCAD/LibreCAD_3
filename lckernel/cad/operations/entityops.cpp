#include "entityops.h"
#include "cad/document/document.h"

#include "cad/document/storagemanager.h"
using namespace lc;
using namespace lc::operation;

/********************************************************************************************************/
/** Base                                                                                              ***/
/********************************************************************************************************/
Begin::Begin() :  Base() {
}

std::vector<entity::CADEntity_CSPtr> Begin::process(
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> operationStack) {
    _entities.insert(_entities.end(), entitySet.begin(), entitySet.end());
    return entitySet;
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
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>& _workingBuffer,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr> _stack) {
    std::vector<entity::CADEntity_CSPtr> final;

    // Find the start
    std::vector<entity::CADEntity_CSPtr> _start;

    for (const auto& base : _stack) {
        //TODO can we get rid of dynamic_cast??
        const lc::operation::Begin* begin = dynamic_cast<const lc::operation::Begin*>(base.get());

        if (begin != nullptr) {
            auto entities = begin->getEntities();
            _start.insert(_start.end(), entities.begin(), entities.end());
        }
    }


    // run the operation queue
    std::vector<entity::CADEntity_CSPtr> entitySet2(entitySet);

    for (int n = 0; n < _numTimes - 1; n++) {
        for (auto base : _stack) {
            entitySet2 = base->process(document, entitySet2, _workingBuffer, removals, _stack);
        }
    }

    return entitySet2;
}

/********************************************************************************************************/
/** Move                                                                                              ***/
/********************************************************************************************************/
Move::Move(const geo::Coordinate& offset) :  Base(), _offset(offset) {
}

std::vector<entity::CADEntity_CSPtr>  Move::process(
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>&,
    std::vector<entity::CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entitySet) {
        auto e = entity->move(_offset);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Copy                                                                                              ***/
/********************************************************************************************************/
Copy::Copy(const geo::Coordinate& offset) : Base(), _offset(offset) {
}

std::vector<entity::CADEntity_CSPtr> Copy::process(
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entitySet) {
        auto e = entity->copy(_offset);
        workingBuffer.push_back(entity);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Scale                                                                                             ***/
/********************************************************************************************************/
Scale::Scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) : Base(), _scale_center(scale_center), _scale_factor(scale_factor) {
}

std::vector<entity::CADEntity_CSPtr> Scale::process(
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>&,
    std::vector<entity::CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entitySet) {
        auto e = entity->scale(_scale_center, _scale_factor);
        newQueue.push_back(e);
    }

    return newQueue;
}

/********************************************************************************************************/
/** Rotate                                                                                            ***/
/********************************************************************************************************/
Rotate::Rotate(const geo::Coordinate& rotation_center, const double rotation_angle) : Base(), _rotation_center(rotation_center), _rotation_angle(rotation_angle) {
}

std::vector<entity::CADEntity_CSPtr> Rotate::process(
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>&,
    std::vector<entity::CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<entity::CADEntity_CSPtr> newQueue;

    for (const auto& entity : entitySet) {
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
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {
    std::vector<entity::CADEntity_CSPtr> newQueue(workingBuffer);
    newQueue.insert(newQueue.end(), entitySet.begin(), entitySet.end());
    workingBuffer.clear();
    return newQueue;
}

/********************************************************************************************************/
/** SelectByLayer                                                                                     ***/
/********************************************************************************************************/
SelectByLayer::SelectByLayer(const Layer_CSPtr& layer) : Base(), _layer(layer) {
}

std::vector<entity::CADEntity_CSPtr> SelectByLayer::process(
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr>,
    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
    std::vector<entity::CADEntity_CSPtr>&,
    const std::vector<Base_SPtr>) {

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
    const std::shared_ptr<Document> document,
    const std::vector<entity::CADEntity_CSPtr> entitySet,
    std::vector<entity::CADEntity_CSPtr>&,
    std::vector<entity::CADEntity_CSPtr>& removals,
    const std::vector<Base_SPtr>) {
    removals.insert(removals.end(), entitySet.begin(), entitySet.end());
    std::vector<entity::CADEntity_CSPtr> e;
    return e;
}