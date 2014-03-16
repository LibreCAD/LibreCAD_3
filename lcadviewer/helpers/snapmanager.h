#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QObject>
#include <QList>

#include "cad/document/abstractdocument.h"
#include "cad/interface/snapable.h"
#include "cad/geometry/geocoordinate.h"
#include "events/snappointevent.h"
#include "events/mousereleaseevent.h"

/*!
 * \brief Snapmanger is a interface to help snapping to grids and entities
 *
 * Other classes can subscribe to the events to get snap positions from the screen
 *
 * \sa lc::SnapManagerImpl
 */
class SnapManager : public QObject {
        Q_OBJECT
    public:
    signals:
        /*!
         * \brief Snap point event get emited when the snap point changed
         *
         * Snap point events gets emitted when the snap positionw as changed or when no snap point was found
         *
         * \sa lc::SnapPointEvent
         */
        void snapPointEvent(const SnapPointEvent&) ;

        /*!
         * \brief mouseRelease event get's emitted when the user release the left mouse button
         *
         * When the user release the left mouse butten this events get's emitted with the coordination
         * of the last snap position. If no snap position was found the current mouse coordinate will be used
         *
         * \sa lc::MouseReleaseEvent
         */
        void mouseReleaseEvent(const MouseReleaseEvent&);

        /*!
         * \brief mouseRelease event get's emitted when the user release the left mouse button
         *
         * When the user release the left mouse butten this events get's emitted with the coordination
         * of the last snap position. If no snap position was found the current mouse coordinate will be used
         *
         * \sa lc::MouseReleaseEvent
         */
        void mouseRightReleaseEvent(const MouseReleaseEvent&);

    public:
        virtual void setGridSnappable(bool gridSnappable) = 0;
        virtual bool isGridSnappable() const = 0;
};


#endif // SNAPMANAGER_H
