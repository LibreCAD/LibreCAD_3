#include "trimoperation.h"

#include "guioperationfinishedevent.h"
#include <typeinfo>

TrimOperation::TrimOperation(lc::Document* document, shared_ptr<lc::StorageManager> entityManager, QGraphicsView* graphicsView, shared_ptr<SnapManager>  snapManager, shared_ptr<lc::SelectionManager> selectionManager) :
    GuiOperation(document), _graphicsView(graphicsView), _snapManager(snapManager), _entityManager(entityManager) {
    connect(graphicsView, SIGNAL(drawEvent(const DrawEvent&)),
            this, SLOT(on_drawEvent(const DrawEvent&)));
    connect(snapManager.get(), SIGNAL(snapPointEvent(const SnapPointEvent&)),
            this, SLOT(on_SnapPoint_Event(const SnapPointEvent&)));


    _trimSelection = new QSnappedState();
    _limitSelections = new QSnappedState();
    QSnappedState* _finishLine = new QSnappedState();
    QFinalState* finishState = new QFinalState();

    connect(_limitSelections, SIGNAL(propertiesAssigned()),
            this, SLOT(on_LimitPropertiesAssigned()));
    connect(_trimSelection, SIGNAL(propertiesAssigned()),
            this, SLOT(on_TrimPropertiesAssigned()));

    _limitSelections->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseReleaseEvent(MouseReleaseEvent)", _limitSelections);
    _limitSelections->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseRightReleaseEvent(MouseReleaseEvent)", _trimSelection);
    _limitSelections->assignMouseReleaseEvent(this, "mouseReleaseEvent");

    _trimSelection->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseReleaseEvent(MouseReleaseEvent)", _finishLine);

    _finishLine->addTransition(finishState);
    _finishLine->assignMouseReleaseEvent(this, "mouseReleaseEvent");
    _finishLine->assignSnapPoint(this, "clickPoint");


    _machine.addState(_limitSelections);
    _machine.addState(_trimSelection);
    _machine.addState(_finishLine);
    _machine.addState(finishState);
    _machine.setInitialState(_limitSelections);
    _machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    _machine.start();

    QObject::connect(&_machine, SIGNAL(finished()), this, SLOT(trimFinished()));
}

void TrimOperation::trimFinished() {
    GuiOperationFinishedEvent of(*this);
    emit guiOperationFinished(of);
}

shared_ptr<lc::operation::DocumentOperation> TrimOperation::operation() const {
    //    shared_ptr<lc::operation::Create> foo = shared_ptr<lc::operation::Create>( new  lc::operation::Create(document(), "0"));
    //    return foo;
    return shared_ptr<lc::operation::DocumentOperation>();
}

void TrimOperation::on_drawEvent(const DrawEvent& event) {
    //event.painter()->drawLine(_startPoint.pointF(), _lastSnapEvent.snapPoint().pointF());

}

void TrimOperation::on_LimitPropertiesAssigned() {
    QList<lc::EntityDistance> enties = this->_mouseReleaseEvent.entities();
    qSort(enties.begin(), enties.end(), lc::EntityDistance::sortAscending);

    if (enties.count() > 0) {
        shared_ptr<const lc::CADEntity> entity = enties.at(0).entity();
        qDebug() << "Select entity with id" << entity->id() << " I am  " << typeid(this).name() << " object was a " << typeid(entity).name();
    }
}
void TrimOperation::on_TrimPropertiesAssigned() {
    QList<lc::EntityDistance> enties = this->_mouseReleaseEvent.entities();
    qSort(enties.begin(), enties.end(), lc::EntityDistance::sortAscending);

    if (enties.count() > 0) {
        shared_ptr<const lc::CADEntity> entity = enties.at(0).entity();
        qDebug() << "Select entity with id" << entity->id() << " I am  " << typeid(this).name() << " object was a " << typeid(entity).name();
    }
}

void TrimOperation::restart() {
    _machine.setInitialState(_limitSelections);
    _machine.start();
}

shared_ptr<GuiOperation> TrimOperation::next() const {
    shared_ptr<GuiOperation> lo = shared_ptr<GuiOperation>(new TrimOperation(document(), _entityManager, _graphicsView, _snapManager, _selectionManager));
    return lo;
}
