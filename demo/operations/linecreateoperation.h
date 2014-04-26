#ifndef LINECREATEOPERATION_H
#define LINECREATEOPERATION_H

#include <QState>
#include <QStateMachine>

#include <cad/document/entitymanager.h>

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
        Q_PROPERTY(lc::geo::Coordinate startPoint READ startPoint WRITE setStartPoint)
        Q_PROPERTY(lc::geo::Coordinate endPoint READ endPoint WRITE setEndPoint)
    public:
        LineCreateOperation(lc::Document* document, shared_ptr<lc::EntityManager> entityManager, shared_ptr<const lc::Layer> layer, QGraphicsView* graphicsView, shared_ptr<SnapManager>  snapManager);

        virtual shared_ptr<lc::operation::DocumentOperation> operation() const;

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

        virtual shared_ptr<GuiOperation> next() const;

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
        shared_ptr<SnapManager>  _snapManager;
        shared_ptr<lc::EntityManager>  _entityManager;
        shared_ptr<const lc::Layer>  _layer;

};

#endif // LINECREATEOPERATION_H
