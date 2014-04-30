#ifndef TRIMOPERATION_H
#define TRIMOPERATION_H

#include <QState>
#include <QStateMachine>

#include "cad/document/storagemanager.h"

#include "guioperation.h"
#include "events/drawevent.h"
#include "events/mousereleaseevent.h"
#include "helpers/snapmanager.h"
#include "cad/document/selectionmanager.h"
#include "drawitems/cursor.h"

#include "qsnappedstate.h"

class TrimOperation : public GuiOperation {
        Q_OBJECT
        Q_PROPERTY(lc::geo::Coordinate clickPoint READ clickPoint WRITE setClickPoint)
        Q_PROPERTY(MouseReleaseEvent mouseReleaseEvent READ mouseReleaseEvent WRITE setMouseReleaseEvent)
    public:
        TrimOperation(lc::Document* document, std::shared_ptr<lc::StorageManager> entityManager, QGraphicsView* graphicsView, std::shared_ptr<SnapManager>  snapManager,  std::shared_ptr<lc::SelectionManager> selectionManager);

    public slots:
        void on_drawEvent(const DrawEvent& event);

        void setClickPoint(const lc::geo::Coordinate& clickPoint) {
            _clickPoint = clickPoint;
        };
        lc::geo::Coordinate clickPoint() const {
            return _clickPoint;
        }
        void setMouseReleaseEvent(const  MouseReleaseEvent& mre) {
            _mouseReleaseEvent = mre;
        };
        MouseReleaseEvent mouseReleaseEvent() const {
            return _mouseReleaseEvent;
        }

        virtual void restart();
        virtual std::shared_ptr<GuiOperation> next() const;
        virtual std::shared_ptr<lc::operation::DocumentOperation> operation() const;
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
        std::shared_ptr<SnapManager>  _snapManager;
        std::shared_ptr<lc::SelectionManager> _selectionManager;
        MouseReleaseEvent _mouseReleaseEvent;

        lc::geo::Coordinate _clickPoint;
        std::shared_ptr<lc::StorageManager>  _entityManager;

};

#endif // TRIMOPERATION_H
