#include "circlecreateoperation.h"

#include "cad/primitive/circle.h"
#include "operationfinishedevent.h"

CircleCreateOperation::CircleCreateOperation(QGraphicsView* graphicsView, SnapManagerPtr snapManager) : Operation(), _graphicsView(graphicsView), _snapManager(snapManager) {
    connect(graphicsView, SIGNAL(drawEvent(const DrawEvent&)),
            this, SLOT(on_drawEvent(const DrawEvent&)));
    connect(snapManager.get(), SIGNAL(snapPointEvent(const SnapPointEvent&)),
            this, SLOT(on_SnapPoint_Event(const SnapPointEvent&)));


    QSnappedState* _waitForFirstClick = new QSnappedState();
    _waitForSecondClick = new QSnappedState();
    QSnappedState* _finishLine = new QSnappedState();
    QFinalState* finishState = new QFinalState();

    _waitForFirstClick->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseReleaseEvent(MouseReleaseEvent)", _waitForSecondClick);
    _waitForFirstClick->assignProperty(this, "currentState", 1);

    _waitForSecondClick->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseReleaseEvent(MouseReleaseEvent)", _finishLine);
    _waitForSecondClick->assignProperty(this, "hasStartPoint", true);
    _waitForSecondClick->assignProperty(this, "currentState", 2);
    _waitForSecondClick->assignSnapPoint(this, "startPoint");

    _finishLine->addTransition(finishState);
    _finishLine->assignProperty(this, "hasEndPoint", true);
    _finishLine->assignSnapPoint(this, "endPoint");
    _finishLine->assignProperty(this, "currentState", 3);

    _machine.addState(_waitForFirstClick);
    _machine.addState(_waitForSecondClick);
    _machine.addState(_finishLine);
    _machine.addState(finishState);
    _machine.setInitialState(_waitForFirstClick);
    _machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    _machine.start();

    QObject::connect(&_machine, SIGNAL(finished()), this, SLOT(lineCreationFinished()));
}

void CircleCreateOperation::lineCreationFinished() {
    OperationFinishedEvent of;
    emit operationFinished(of);
}

void CircleCreateOperation::restart() {
    _machine.setInitialState(_waitForFirstClick);
    _machine.start();
}

lc::CADEntityPtr CircleCreateOperation::cadEntity(const QList<lc::MetaTypePtr>& metaTypes) const {
    double r = (lc::geo::Coordinate(_startPoint) - lc::geo::Coordinate(_lastSnapEvent.snapPoint())).magnitude();
    return lc::CirclePtr(new lc::Circle(_startPoint, r, metaTypes));
}

void CircleCreateOperation::on_drawEvent(const DrawEvent& event) {
    bool s = property("hasStartPoint").toBool();
    bool e = property("hasEndPoint").toBool();

    if (s == true && e == false) {
        event.painter()->drawLine(_startPoint.pointF(), _lastSnapEvent.snapPoint().pointF());
        double r = (lc::geo::Coordinate(_startPoint) - lc::geo::Coordinate(_lastSnapEvent.snapPoint())).magnitude();
        event.painter()->drawEllipse(_startPoint.pointF(), r, r);
    }

    if (s == true && e == true) {
        double r = (lc::geo::Coordinate(_startPoint) - lc::geo::Coordinate(_endPoint)).magnitude();
        event.painter()->drawEllipse(_startPoint.pointF(), r, r);
    }

}

void CircleCreateOperation::on_SnapPoint_Event(const SnapPointEvent& event) {
    _lastSnapEvent = event;
}


OperationPtr CircleCreateOperation::next() const {
    // Create a new line end set the start point to the end point of the last operation
    CircleCreateOperation* lco = new CircleCreateOperation(this->_graphicsView, this->_snapManager);
    return OperationPtr(lco);
}
