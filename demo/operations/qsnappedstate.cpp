#include "qsnappedstate.h"

#include <QDebug>
#include <QStateMachine>
#include "events/mousereleaseevent.h"

QSnappedState::QSnappedState() {
    _spobject = NULL;
    _mreobject = NULL;
}

void QSnappedState::onEntry(QEvent* event) {
    QStateMachine::SignalEvent* se = static_cast<QStateMachine::SignalEvent*>(event);

    if (event->type() != QEvent::None && se->arguments().count() > 0) {
        //FIXME: For some reason the first click is a empty event!?!?!
        MouseReleaseEvent mre = se->arguments().at(0).value<MouseReleaseEvent>();

        if (_spobject != NULL) {
            QVariant v;
            v.setValue(mre.mousePosition());
            _spobject->setProperty(_spname, v);
        }

        if (_mreobject != NULL) {
            QVariant v;
            v.setValue(mre);
            _mreobject->setProperty(_mrename, v);
        }
    }

    QState::onEntry(event);
}

void QSnappedState::assignSnapPoint(QObject* object, char* name) {
    _spobject = object;
    _spname = name;
}
void QSnappedState::assignMouseReleaseEvent(QObject* object, char* name) {
    _mreobject = object;
    _mrename = name;
}
