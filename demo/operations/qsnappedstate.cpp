#include "qsnappedstate.h"

#include <QDebug>
#include <QStateMachine>
#include "events/mousereleaseevent.h"

QSnappedState::QSnappedState() {
    _object = NULL;
}

void QSnappedState::onEntry(QEvent* event) {
    QStateMachine::SignalEvent* se = static_cast<QStateMachine::SignalEvent*>(event);

    if (event->type() != QEvent::None && se->arguments().count() > 0) {
        MouseReleaseEvent mre = se->arguments().at(0).value<MouseReleaseEvent>();

        if (_object != NULL) {
            QVariant v;
            v.setValue(mre.mousePosition());
            _object->setProperty(_name, v);
        }
    }

    QState::onEntry(event);
}

void QSnappedState::assignSnapPoint(QObject* object, char* name) {
    _object = object;
    _name = name;
}
