#pragma once

#include <QState>
#include <QStateMachine>

#include "cad/document/storagemanager.h"

#include "guioperation.h"
#include "events/drawevent.h"
#include "events/mousereleaseevent.h"
#include <managers/snapmanager.h>
#include <drawables/lccursor.h>

#include "qsnappedstate.h"

class TrimOperation : public GuiOperation {
        Q_OBJECT
    public:
        TrimOperation(std::shared_ptr<lc::Document> document, lc::StorageManager_SPtr entityManager, QGraphicsView* graphicsView, LCViewer::SnapManager_SPtr  snapManager);

    public slots:
        void on_drawEvent(const LCViewer::DrawEvent& event);

        void setClickPoint(const lc::geo::Coordinate& clickPoint) {
            _clickPoint = clickPoint;
        };
        lc::geo::Coordinate clickPoint() const {
            return _clickPoint;
        }
        void setMouseReleaseEvent(const  LCViewer::MouseReleaseEvent& mre) {
            _mouseReleaseEvent = mre;
        };
        LCViewer::MouseReleaseEvent mouseReleaseEvent() const {
            return _mouseReleaseEvent;
        }

        virtual void restart();
        virtual GuiOperation_SPtr next() const;
        virtual lc::operation::DocumentOperation_SPtr operation() const;
    private slots:
        void trimFinished();
        void on_LimitPropertiesAssigned();
        void on_TrimPropertiesAssigned();

    private:
        // Needed for state machine
        QStateMachine _machine;
        QSnappedState* _limitSelections;
        QSnappedState* _trimSelection;

        // needed to find the correct information sets
        QGraphicsView* _graphicsView;
        LCViewer::SnapManager_SPtr  _snapManager;
        LCViewer::MouseReleaseEvent _mouseReleaseEvent;

        lc::geo::Coordinate _clickPoint;
        lc::StorageManager_SPtr  _entityManager;

};

// TRIMOPERATION_H
