#include "builderops.h"
#include "cad/document/abstractdocument.h"


using namespace lc;
using namespace lc::operation;




Begin::Begin() :  Base() {
}

QList<shared_ptr<const CADEntity> > Begin::process(
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
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& buff,
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
            newQueue = _stack.at(i)->process(newQueue, buff, _stack);
        }
    }

    return newQueue;
}



Move::Move(const geo::Coordinate& offset) :  Base(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> >  Move::process(
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
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& buf,
    const QList<shared_ptr< Base> >) {
    QList<shared_ptr<const CADEntity> > newQueue;

    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->copy(_offset);
        buf.append(entities.at(i));
        newQueue.append(e);
    }

    return newQueue;
}



Rotate::Rotate(const geo::Coordinate& rotation_center, const double rotation_angle) : Base(), _rotation_center(rotation_center), _rotation_angle(rotation_angle) {
}


QList<shared_ptr<const CADEntity> > Rotate::process(
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
    QList<shared_ptr<const CADEntity> > entities,
    QList<shared_ptr<const CADEntity> >& buf,
    const QList<shared_ptr< Base> >) {
    QList<shared_ptr<const CADEntity> > newQueue(buf);
    newQueue.append(entities);
    buf.clear();
    return newQueue;
}

