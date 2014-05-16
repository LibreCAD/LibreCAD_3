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
        TrimOperation(lc::Document* document, lc::StorageManager_SPtr entityManager, QGraphicsView* graphicsView, SnapManager_SPtr  snapManager,  std::shared_ptr<lc::SelectionManager> selectionManager);

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
        SnapManager_SPtr  _snapManager;
        std::shared_ptr<lc::SelectionManager> _selectionManager;
        MouseReleaseEvent _mouseReleaseEvent;

        lc::geo::Coordinate _clickPoint;
        lc::StorageManager_SPtr  _entityManager;

};

#endif // TRIMOPERATION_H
