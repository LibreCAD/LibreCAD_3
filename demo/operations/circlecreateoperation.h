#ifndef CIRCLECREATEOPERATION_H
#define CIRCLECREATEOPERATION_H

#include <QState>
#include <QStateMachine>

#include <cad/meta/layer.h>

#include "cad/document/storagemanager.h"

#include "guioperation.h"
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
class CircleCreateOperation :  public GuiOperation {
        Q_OBJECT
        Q_PROPERTY(lc::geo::Coordinate startPoint READ startPoint WRITE setStartPoint)
        Q_PROPERTY(lc::geo::Coordinate endPoint READ endPoint WRITE setEndPoint)
    public:
        CircleCreateOperation(lc::Document* document, lc::StorageManager_SPtr storageManager, lc::Layer_CSPtr layer, QGraphicsView* graphicsView, SnapManager_SPtr  snapManager);

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

        virtual GuiOperation_SPtr next() const;

        void on_SnapPoint_Event(const SnapPointEvent& event);

        virtual lc::operation::DocumentOperation_SPtr operation() const;
    private slots:
        void circleCreationFinished();

    private:
        lc::geo::Coordinate _startPoint;
        lc::geo::Coordinate _endPoint;

        QStateMachine _machine;
        QSnappedState* _waitForSecondClick;
        QSnappedState* _waitForFirstClick;

        SnapPointEvent _lastSnapEvent;

        QGraphicsView* _graphicsView;
        SnapManager_SPtr  _snapManager;
        lc::StorageManager_SPtr  _storageManager;
        lc::Layer_CSPtr _layer;

};

#endif // CircleCreateOperation_H
