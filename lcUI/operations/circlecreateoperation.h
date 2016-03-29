#pragma once

#include <QState>
#include <QStateMachine>
#include <cairo/cairo.h>
#include <cad/meta/layer.h>

#include "cad/document/storagemanager.h"

#include "guioperation.h"
#include "events/drawevent.h"
#include "events/mousereleaseevent.h"
#include "managers/snapmanager.h"
#include "drawables/lccursor.h"

#include "qsnappedstate.h"

/*!
  * \brief Create a circle on a center point
  *
  * \note Might need to get changed name so it won't conflict with other circle creation operations
  */
class CircleCreateOperation :  public GuiOperation {
        Q_OBJECT
    public:
        CircleCreateOperation(std::shared_ptr<lc::Document>, lc::StorageManager_SPtr storageManager, lc::Layer_CSPtr layer, QGraphicsView* graphicsView, LCViewer::SnapManager_SPtr  snapManager);

        virtual void restart();

    public slots:
        void on_drawEvent(const LCViewer::DrawEvent& event);

        void setStartPoint(const lc::geo::Coordinate& startPoint) {
            _startPoint = startPoint;
        }
        void setEndPoint(const lc::geo::Coordinate& endPoint) {
            _endPoint = endPoint;
        }

        lc::geo::Coordinate startPoint() const {
            return _startPoint;
        }
        lc::geo::Coordinate  endPoint() const {
            return _endPoint;
        }

        virtual GuiOperation_SPtr next() const;

        void on_SnapPoint_Event(const LCViewer::SnapPointEvent& event);

        virtual lc::operation::DocumentOperation_SPtr operation() const;
    private slots:
        void circleCreationFinished();

    private:
        lc::geo::Coordinate _startPoint;
        lc::geo::Coordinate _endPoint;

        QStateMachine _machine;
        QSnappedState* _waitForSecondClick;
        QSnappedState* _waitForFirstClick;

        LCViewer::SnapPointEvent _lastSnapEvent;

        QGraphicsView* _graphicsView;
        LCViewer::SnapManager_SPtr  _snapManager;
        lc::StorageManager_SPtr  _storageManager;
        lc::Layer_CSPtr _layer;

};
