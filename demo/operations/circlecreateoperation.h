#ifndef CIRCLECREATEOPERATION_H
#define CIRCLECREATEOPERATION_H

#include <QState>
#include <QStateMachine>

#include "operation.h"
#include "events/drawevent.h"
#include "events/mousereleaseevent.h"
#include "helpers/snapmanager.h"
#include "drawitems/cursor.h"

#include "qsnappedstate.h"

/*!
  * \brief Create a circle on a center point
  *
  * \note Might need to get changed name so it won't conflict with other circle creation operations
  */
class CircleCreateOperation : public Operation {
        Q_OBJECT
        Q_PROPERTY(lc::geo::Coordinate startPoint READ startPoint WRITE setStartPoint)
        Q_PROPERTY(lc::geo::Coordinate endPoint READ endPoint WRITE setEndPoint)
    public:
        CircleCreateOperation(QGraphicsView* graphicsView, SnapManagerPtr snapManager);

        virtual lc::CADEntityPtr cadEntity(const QList<lc::MetaTypePtr>& metaTypes) const;

        virtual void restart();

    public slots:
        void on_drawEvent(const DrawEvent& event);

        void setStartPoint(const lc::geo::Coordinate& startPoint) {
            _startPoint = startPoint;
        };
        void setEndPoint(const lc::geo::Coordinate& endPoint) {
            _endPoint = endPoint;
        };

        lc::geo::Coordinate startPoint() const {
            return _startPoint;
        }
        lc::geo::Coordinate  endPoint() const {
            return _endPoint;
        }

        virtual OperationPtr next() const;

        void on_SnapPoint_Event(const SnapPointEvent& event);
    private slots:
        void lineCreationFinished();

    private:
        lc::geo::Coordinate _startPoint;
        lc::geo::Coordinate _endPoint;

        QStateMachine _machine;
        QSnappedState* _waitForSecondClick;
        QSnappedState* _waitForFirstClick;

        SnapPointEvent _lastSnapEvent;

        QGraphicsView* _graphicsView;
        SnapManagerPtr _snapManager;
};

#endif // CircleCreateOperation_H
