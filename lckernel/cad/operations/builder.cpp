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
    _stack.append(make_shared<const BMove>(offset));
    return *this;
}
Builder& Builder::copy(const geo::Coordinate& offset) {
    _stack.append(make_shared<const BCopy>(offset));
    return *this;
}
Builder& Builder::repeat(const int numTimes) {
    _stack.append(make_shared<const BRepeat>(numTimes));
    return *this;
}

void Builder::processInternal() {
    QList<shared_ptr<const CADEntity> > newQueue;
    newQueue.append(_operationQue);
    for (int i = 0; i < _stack.size(); ++i) {
        QList<shared_ptr<const BBase> > sup = _stack.mid(0, i);
        newQueue = _stack.at(i)->process(newQueue, sup);
    }
    _operationFinal.clear();
    _operationFinal.append(newQueue);

    for (int i = 0; i < _operationQue.size(); ++i) {
        document()->removeEntity(_operationQue.at(i)->id());
    }
    for (int i = 0; i < _operationFinal.size(); ++i) {
        document()->addEntity("0", _operationFinal.at(i));
    }
}

void Builder::undo() const {
    for (int i = 0; i < _operationFinal.size(); ++i) {
        document()->removeEntity(_operationFinal.at(i)->id());
    }
    for (int i = 0; i < _operationQue.size(); ++i) {
        document()->addEntity("0", _operationQue.at(i));
    }
}
void Builder::redo() const {
    for (int i = 0; i < _operationQue.size(); ++i) {
        document()->removeEntity(_operationQue.at(i)->id());
    }
    for (int i = 0; i < _operationFinal.size(); ++i) {
        document()->addEntity("0", _operationFinal.at(i));
    }
}



BRepeat::BRepeat( const int numTimes) :  BBase(), _numTimes(numTimes) {
}

QList<shared_ptr<const CADEntity> > BRepeat::process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _operationQue) const {
    QList<shared_ptr<const CADEntity> > final;
    final.append(entities);
    for (int n = 0; n<_numTimes-1;n++) {
        for (int i = 0; i < _operationQue.size(); ++i) {
            final = _operationQue.at(i)->process(final, _operationQue);
        }
    }

    return final;
}





BMove::BMove( const geo::Coordinate& offset) :  BBase(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> > BMove::process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _operationQue) const {
    QList<shared_ptr<const CADEntity> > newQueue;
    for (int i = 0; i < entities.size(); ++i) {
        newQueue.append(entities.at(i)->move(_offset));
    }
    return newQueue;
}





BCopy::BCopy( const geo::Coordinate& offset) : BBase(), _offset(offset) {
}

QList<shared_ptr<const CADEntity> > BCopy::process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _operationQue) const {
    QList<shared_ptr<const CADEntity> > newQueue;
    newQueue.append(entities);
    for (int i = 0; i < entities.size(); ++i) {
        newQueue.append(entities.at(i)->copy(_offset));
    }
    return newQueue;
}


