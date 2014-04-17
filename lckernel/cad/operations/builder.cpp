#include "builder.h"
#include "cad/document/abstractdocument.h"


using namespace lc;
using namespace lc::operation;

Builder::Builder(AbstractDocument* document) : Operation(document), Undoable("Builder")
{
}

Builder& Builder::append(shared_ptr<const CADEntity> cadEntity) {
    _operationQue.append(cadEntity);
    return *this;
}



Builder& Builder::move(const geo::Coordinate& offset) {
    _stack.append(make_shared< BMove>(offset));
    return *this;
}
Builder& Builder::copy(const geo::Coordinate& offset) {
    _stack.append(make_shared< BCopy>(offset));
    return *this;
}
Builder& Builder::repeat(const int numTimes) {
    _stack.append(make_shared< BRepeat>(numTimes));
    return *this;
}
Builder& Builder::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) {
    _stack.append(make_shared< BRotate>(rotation_center, rotation_angle));
    return *this;
}
Builder& Builder::begin() {
    _stack.append(make_shared< BBegin>());
    return *this;
}

void Builder::processInternal() {
    QList<shared_ptr<const CADEntity> > newQueue(_operationQue);
    for (int i = 0; i < _stack.size(); ++i) {
        QList<shared_ptr<BBase> > sup = _stack.mid(0, i);
        newQueue= _stack.at(i)->process(newQueue, _inserts, _updates, _deletes, sup);
    }
    _operationFinal.append(newQueue);

    qDebug() << "_deletes : " << _deletes.size();
    qDebug() << "_inserts : " << _inserts.size();
    qDebug() << "_updates : " << _updates.size();

    for (int i = 0; i < _inserts.size(); ++i) {
        document()->addEntity("0", _inserts.at(i));
    }
    for (int i = 0; i < _updates.size(); ++i) {
        document()->replaceEntity( _inserts.at(i),  _inserts.at(i));
    }
    for (int i = 0; i < _deletes.size(); ++i) {
        document()->removeEntity(_deletes.at(i)->id());
    }
}

void Builder::undo() const {

}
void Builder::redo() const {

}



BBegin::BBegin( ) :  BBase() {
}

QList<shared_ptr<const CADEntity> > BBegin::process(
        QList<shared_ptr<const CADEntity> > entities,
        QList<shared_ptr<const CADEntity> > &inserts,
        QList<shared_ptr<const CADEntity> > &updates,
        QList<shared_ptr<const CADEntity> > &deletes,
        const QList<shared_ptr< BBase> > _stack) {
    _entities.append(entities);
    return entities;
}

QList<shared_ptr<const CADEntity> > BBegin::getEntities() const {
    return _entities;
}



BRepeat::BRepeat( const int numTimes) :  BBase(), _numTimes(numTimes) {
}

QList<shared_ptr<const CADEntity> > BRepeat::process(
        QList<shared_ptr<const CADEntity> > entities,
        QList<shared_ptr<const CADEntity> > &inserts,
        QList<shared_ptr<const CADEntity> > &updates,
        QList<shared_ptr<const CADEntity> > &deletes,
        const QList<shared_ptr< BBase> > _stack) {
QList<shared_ptr<const CADEntity> > final;

/*
    // Find the start
    QList<shared_ptr<const CADEntity> > _start;
    for (int i = 0; i < _operationQue.size(); ++i) {
        const lc::operation::BBegin* begin = dynamic_cast<const lc::operation::BBegin*>(_operationQue.at(i).get());
        if (begin!=NULL) {
            _start.append(begin->getEntities());
        }
    }

    // run the operation que
    for (int n = 0; n<_numTimes-1;n++) {
        for (int i = 0; i < _operationQue.size(); ++i) {
            final.append(_operationQue.at(i)->process(_start, toRemove,  _operationQue));
        }
    } */

    return final;
}



BMove::BMove( const geo::Coordinate& offset) :  BBase(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> >  BMove::process(
        QList<shared_ptr<const CADEntity> > entities,
        QList<shared_ptr<const CADEntity> > &inserts,
        QList<shared_ptr<const CADEntity> > &updates,
        QList<shared_ptr<const CADEntity> > &deletes,
        const QList<shared_ptr< BBase> > _stack) {
QList<shared_ptr<const CADEntity> > newQueue;
    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->move(_offset);
        newQueue.append(e);
        updates.append(e);
    }
    return newQueue;
}





BCopy::BCopy( const geo::Coordinate& offset) : BBase(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> > BCopy::process(
        QList<shared_ptr<const CADEntity> > entities,
        QList<shared_ptr<const CADEntity> > &inserts,
        QList<shared_ptr<const CADEntity> > &updates,
        QList<shared_ptr<const CADEntity> > &deletes,
        const QList<shared_ptr< BBase> > _stack) {
    QList<shared_ptr<const CADEntity> > newQueue;
    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->copy(_offset);
        newQueue.append(e);
        inserts.append(e);
    }
    qDebug() <<  "BCopy" << newQueue.size();
    return newQueue;
}



BRotate::BRotate( const geo::Coordinate& rotation_center, const double rotation_angle) : BBase(), _rotation_center(rotation_center), _rotation_angle(rotation_angle) {
}


QList<shared_ptr<const CADEntity> > BRotate::process(
        QList<shared_ptr<const CADEntity> > entities,
        QList<shared_ptr<const CADEntity> > &inserts,
        QList<shared_ptr<const CADEntity> > &updates,
        QList<shared_ptr<const CADEntity> > &deletes,
        const QList<shared_ptr< BBase> > _stack) {
QList<shared_ptr<const CADEntity> > newQueue;
    for (int i = 0; i < entities.size(); ++i) {
        auto e = entities.at(i)->rotate(_rotation_center, _rotation_angle);
        newQueue.append(e);
        updates.append(e);
    }
    return newQueue;
}


