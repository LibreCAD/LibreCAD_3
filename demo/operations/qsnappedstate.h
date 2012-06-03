#ifndef QSNAPPEDSTATE_H
#define QSNAPPEDSTATE_H

#include <QState>
#include <QAbstractState>
#include <QVariant>
#include <QString>

/*!
  * \brief QSnappedState can be used to fetch Coordinates and assign them as properties to your class
  *
  *
  * \code
  * _waitForFirstClick = new QSnappedState();
  * QSnappedState* _waitForSecondClick = new QSnappedState();
  * QSnappedState* _finishLine = new QSnappedState();
  * QFinalState* finishState = new QFinalState();

  * // Wait for the user to first click
  * _waitForFirstClick->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseReleaseEvent(MouseReleaseEvent)", _waitForSecondClick);

  * // When the first click was done, we save it when we enter this state, also wait for the second click
    _waitForSecondClick->addTransition(qobject_cast<SnapManager*>(snapManager.get()),  "mouseReleaseEvent(MouseReleaseEvent)", _finishLine);
  * _waitForSecondClick->assignProperty(this, "hasStartPoint", true);
  * _waitForSecondClick->assignSnapPoint(this, "startPoint");

  * // When the user clicked a second time we assign the property and go directly to the finnish state
  * _finishLine->addTransition(finishState);
  * _finishLine->assignProperty(this, "hasEndPoint", true);
  * _finishLine->assignSnapPoint(this, "endPoint");

  * _machine.addState(_waitForFirstClick);
  * _machine.addState(_waitForSecondClick);
  * _machine.addState(_finishLine);
  * _machine.addState(finishState);
  * _machine.setInitialState(_waitForFirstClick);
  * _machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
  * _machine.start();
  *
  * // When this operation is finnished called lineCreationFinished, whish in return will send a GuiOperationFinishedEvent
  * QObject::connect(&_machine, SIGNAL(finished()), this, SLOT(lineCreationFinished()));
  *
  * \endcode
  */
class QSnappedState : public QState {
    public:
        QSnappedState();
        /*!
          * \brief When this state enters, we assign the snapped point whish is the lc::geo::Coordinate to the property
          *
          * only one propery can be assigned (unlike assignProperty that can handle multiple properties)
          *
          * \param QObject object name with the property
          * \param char* Name of the property
          * \deprecated may be we should use assignMouseReleaseEvent always and get rid of this one, although convenient for drawing routines to get the drawing point directly
          *
        */        
        virtual void assignSnapPoint(QObject* object,  char* name);
        virtual void assignMouseReleaseEvent(QObject* object,  char* name);

    protected:
        virtual void onEntry(QEvent* event);

    protected:
        QObject* _spobject;
        const char* _spname;
        QObject* _mreobject;
        const char* _mrename;
};

#endif // QSNAPPEDSTATE_H
