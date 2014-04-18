#include "builder.h"
#include "cad/document/abstractdocument.h"


using namespace lc;
using namespace lc::operation;

Builder::Builder(AbstractDocument* document) : DocumentOperation(document), Undoable("Builder") {
}

Builder::~Builder() {
    qDebug() << "Builder removed";
}

Builder& Builder::append(shared_ptr<const CADEntity> cadEntity) {
    _operationQue.append(cadEntity);
    return *this;
}

Builder& Builder::move(const geo::Coordinate& offset) {
    _stack.append(make_shared<Move>(offset));
    return *this;
}
Builder& Builder::copy(const geo::Coordinate& offset) {
    _stack.append(make_shared<Copy>(offset));
    return *this;
}
Builder& Builder::repeat(const int numTimes) {
    _stack.append(make_shared<Loop>(numTimes));
    return *this;
}
Builder& Builder::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) {
    _stack.append(make_shared<Rotate>(rotation_center, rotation_angle));
    return *this;
}
Builder& Builder::begin() {
    _stack.append(make_shared<Begin>());
    return *this;
}
Builder& Builder::push() {
    _stack.append(make_shared<Push>());
    return *this;
}

void Builder::processInternal() {
    QList<shared_ptr<const CADEntity> > newQueue(_operationQue);

    for (int i = 0; i < _stack.size(); ++i) {
        QList<shared_ptr<Base> > sup = _stack.mid(0, i);
        newQueue = _stack.at(i)->process(newQueue, _buffer, sup);
    }

    _buffer.append(newQueue);

    for (int i = 0; i < _buffer.size(); ++i) {
        auto exists = document()->findEntityByID(_buffer.at(i)->id());

        if (exists.get() != NULL) {
            _entitiesStart.append(exists);
            document()->replaceEntity(_buffer.at(i), _buffer.at(i));
        } else {
            document()->addEntity("0", _buffer.at(i));
        }
    }
}

void Builder::undo() const {
    for (int i = 0; i < _buffer.size(); ++i) {
        document()->removeEntity(_buffer.at(i)->id());
    }

    for (int i = 0; i < _entitiesStart.size(); ++i) {
        document()->addEntity("0", _entitiesStart.at(i));
    }
}

void Builder::redo() const {
    for (int i = 0; i < _buffer.size(); ++i) {
        auto exists = document()->findEntityByID(_buffer.at(i)->id());

        if (exists.get() != NULL) {
            document()->replaceEntity(_buffer.at(i), _buffer.at(i));
        } else {
            document()->addEntity("0", _buffer.at(i));
        }
    }
}
