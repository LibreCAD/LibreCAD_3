#include "builderops.h"
#include "cad/document/document.h"

#include "cad/document/storagemanager.h"


using namespace lc;
using namespace lc::operation;




Begin::Begin() :  Base() {
}

QList<CADEntity_CSPtr> Begin::process(
    StorageManager_SPtr storageManager,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>& workingBuffer,
    QList<CADEntity_CSPtr>& removals,
    const QList<Base_SPtr>) {
    _entities.append(entitySet);
    return entitySet;
}

QList<CADEntity_CSPtr> Begin::getEntities() const {
    return _entities;
}



Loop::Loop(const int numTimes) :  Base(), _numTimes(numTimes) {
}

QList<CADEntity_CSPtr> Loop::process(
    StorageManager_SPtr storageManager,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>& _workingBuffer,
    QList<CADEntity_CSPtr>& removals,
    const QList<Base_SPtr> _stack) {
    QList<CADEntity_CSPtr> final;

    // Find the start
    QList<CADEntity_CSPtr> _start;

    for (int i = 0; i < _stack.size(); ++i) {
        const lc::operation::Begin* begin = dynamic_cast<const lc::operation::Begin*>(_stack.at(i).get());

        if (begin != nullptr) {
            _start.append(begin->getEntities());
        }
    }


    // run the operation que
    QList<CADEntity_CSPtr> entitySet2(entitySet);

    for (int n = 0; n < _numTimes - 1; n++) {
        for (int i = 0; i < _stack.size(); ++i) {
            entitySet2 = _stack.at(i)->process(storageManager, entitySet2, _workingBuffer, removals, _stack);
        }
    }

    return entitySet2;
}



Move::Move(const geo::Coordinate& offset) :  Base(), _offset(offset) {
}

QList<CADEntity_CSPtr>  Move::process(
    StorageManager_SPtr ,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>&,
    QList<CADEntity_CSPtr>&,
    const QList<Base_SPtr>) {
    QList<CADEntity_CSPtr> newQueue;

    for (int i = 0; i < entitySet.size(); ++i) {
        auto e = entitySet.at(i)->move(_offset);
        newQueue.append(e);
    }

    return newQueue;
}





Copy::Copy(const geo::Coordinate& offset) : Base(), _offset(offset) {
}

QList<CADEntity_CSPtr> Copy::process(
    StorageManager_SPtr ,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>& workingBuffer,
    QList<CADEntity_CSPtr>&,
    const QList<Base_SPtr>) {
    QList<CADEntity_CSPtr> newQueue;

    for (int i = 0; i < entitySet.size(); ++i) {
        auto e = entitySet.at(i)->copy(_offset);
        workingBuffer.append(entitySet.at(i));
        newQueue.append(e);
    }

    return newQueue;
}

Scale::Scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) : Base(), _scale_center(scale_center), _scale_factor(scale_factor) {
}

QList<CADEntity_CSPtr> Scale::process(
    StorageManager_SPtr ,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>&,
    QList<CADEntity_CSPtr>&,
    const QList<Base_SPtr>) {
    QList<CADEntity_CSPtr> newQueue;

    for (int i = 0; i < entitySet.size(); ++i) {
        auto e = entitySet.at(i)->scale(_scale_center, _scale_factor);
        newQueue.append(e);
    }

    return newQueue;
}

Rotate::Rotate(const geo::Coordinate& rotation_center, const double rotation_angle) : Base(), _rotation_center(rotation_center), _rotation_angle(rotation_angle) {
}

QList<CADEntity_CSPtr> Rotate::process(
    StorageManager_SPtr ,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>&,
    QList<CADEntity_CSPtr>&,
    const QList<Base_SPtr>) {
    QList<CADEntity_CSPtr> newQueue;

    for (int i = 0; i < entitySet.size(); ++i) {
        auto e = entitySet.at(i)->rotate(_rotation_center, _rotation_angle);
        newQueue.append(e);
    }

    return newQueue;
}


Push::Push() : Base() {
}

QList<CADEntity_CSPtr> Push::process(
    StorageManager_SPtr ,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>& workingBuffer,
    QList<CADEntity_CSPtr>&,
    const QList<Base_SPtr>) {
    QList<CADEntity_CSPtr> newQueue(workingBuffer);
    newQueue.append(entitySet);
    workingBuffer.clear();
    return newQueue;
}




SelectByLayer::SelectByLayer(const Layer_CSPtr layer) : Base(), _layer(layer) {
}

QList<CADEntity_CSPtr> SelectByLayer::process(
    StorageManager_SPtr storageManager,
    QList<CADEntity_CSPtr> ,
    QList<CADEntity_CSPtr>&,
    QList<CADEntity_CSPtr>&,
    const QList<Base_SPtr>) {
    EntityContainer c = storageManager->entitiesByLayer(_layer);
    QList<CADEntity_CSPtr> e(c.allEntities().values());
    qDebug() << "SelectByLayer::process " << e.size();
    return e;
}




Remove::Remove() : Base() {
}

QList<CADEntity_CSPtr> Remove::process(
    StorageManager_SPtr storageManager,
    QList<CADEntity_CSPtr> entitySet,
    QList<CADEntity_CSPtr>&,
    QList<CADEntity_CSPtr>& removals,
    const QList<Base_SPtr>) {
    removals.append(entitySet);
    QList<CADEntity_CSPtr> e;
    return e;
}
