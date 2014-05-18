#ifndef LINECREATEOPERATION_H
#define LINECREATEOPERATION_H

#include <QState>
#include <QStateMachine>

#include <cad/document/storagemanager.h>

#include <cad/meta/layer.h>
#include "cad/operations/documentoperation.h"

#include "guioperation.h"
#include "events/drawevent.h"
#include "events/mousereleaseevent.h"
#include "helpers/snapmanager.h"
#include "drawitems/cursor.h"

#include "qsnappedstate.h"

/*!
 * \brief Starts a operation to create a new line of the type lc::Line
 *
 *
 */
class LineCreateOperation : public GuiOperation {
        Q_OBJECT
    public:
        LineCreateOperation(lc::Document* document, lc::StorageManager_SPtr entityManager, lc::Layer_CSPtr layer, QGraphicsView* graphicsView, SnapManager_SPtr  snapManager);

        virtual lc::operation::DocumentOperation_SPtr operation() const;

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
        SnapManager_SPtr  _snapManager;
        lc::StorageManager_SPtr  _storageManager;
        lc::Layer_CSPtr  _layer;

};

#endif // LINECREATEOPERATION_H
