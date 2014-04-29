#include "builderops.h"
#include "cad/document/document.h"

#include "cad/document/storagemanager.h"


using namespace lc;
using namespace lc::operation;




Begin::Begin() :  Base() {
}

QList<shared_ptr<const CADEntity> > Begin::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >&,
    const QList<shared_ptr< Base> >) {
    _entities.append(entities);
    return entities;
}

QList<shared_ptr<const CADEntity> > Begin::getEntities() const {
    return _entities;
}



Loop::Loop(const int numTimes) :  Base(), _numTimes(numTimes) {
}

QList<shared_ptr<const CADEntity> > Loop::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& _workingBuffer,
    const QList<shared_ptr< Base> > _stack) {
    QList<shared_ptr<const CADEntity> > final;

    // Find the start
    QList<shared_ptr<const CADEntity> > _start;

    for (int i = 0; i < _stack.size(); ++i) {
        const lc::operation::Begin* begin = dynamic_cast<const lc::operation::Begin*>(_stack.at(i).get());

        if (begin != NULL) {
            _start.append(begin->getEntities());
        }
    }


    // run the operation que
    QList<shared_ptr<const CADEntity> > newQueue(entities);

    for (int n = 0; n < _numTimes - 1; n++) {
        for (int i = 0; i < _stack.size(); ++i) {
            newQueue = _stack.at(i)->process(storageManager, newQueue, _workingBuffer, _stack);
        }
    }

    return newQueue;
}



Move::Move(const geo::Coordinate& offset) :  Base(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> >  Move::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >&,
    const QList<shared_ptr< Base> >) {
    QList<shared_ptr<const CADEntity> > newQueue;

    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->move(_offset);
        newQueue.append(e);
    }

    return newQueue;
}





Copy::Copy(const geo::Coordinate& offset) : Base(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> > Copy::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& workingBuffer,
    const QList<shared_ptr< Base> >) {
    QList<shared_ptr<const CADEntity> > newQueue;

    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->copy(_offset);
        workingBuffer.append(entities.at(i));
        newQueue.append(e);
    }

    return newQueue;
}



Rotate::Rotate(const geo::Coordinate& rotation_center, const double rotation_angle) : Base(), _rotation_center(rotation_center), _rotation_angle(rotation_angle) {
}

QList<shared_ptr<const CADEntity> > Rotate::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >&,
    const QList<shared_ptr< Base> >) {
    QList<shared_ptr<const CADEntity> > newQueue;

    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->rotate(_rotation_center, _rotation_angle);
        newQueue.append(e);
    }

    return newQueue;
}


Push::Push() : Base() {
}

QList<shared_ptr<const CADEntity> > Push::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& workingBuffer,
    const QList<shared_ptr< Base> >) {
    QList<shared_ptr<const CADEntity> > newQueue(workingBuffer);
    newQueue.append(entities);
    workingBuffer.clear();
    return newQueue;
}




SelectByLayer::SelectByLayer(const shared_ptr<const Layer> layer) : Base(), _layer(layer) {
}

QList<shared_ptr<const CADEntity> > SelectByLayer::process(
    shared_ptr<StorageManager> storageManager,
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& ,
    const QList<shared_ptr< Base> >) {
    EntityContainer c = storageManager->entitiesByLayer(_layer);
    QList<shared_ptr<const CADEntity> > e(c.allEntities().values());
    qDebug() << "SelectByLayer::process " << e.size();
    return e;
}


