#pragma once

#include <QObject>
#include <cad/document/selectionmanager.h>
#include "snapmanager.h"
#include "lcadviewer.h"


/*!
 * \brief Implements the SnapManager interface
 */
class SnapManagerImpl : public SnapManager {
        Q_OBJECT
    public:
        /*!
         * \brief Implementation of the SnapManager
         *
         * \param viewer    Pointer to a LCADViewer object
         * \param selectionmanager    SelectionManager object, this allows SnapManager to locate objects within the document
         * \param grid  Grid that can be snapped into
         * \param distanceToSnap  The minimum distance for a entity to get snapped into. Distance given in real coordinates, not screen pixels
         *
         * \sa lc::SnapManager
         */
        SnapManagerImpl(LCADViewer* view, lc::Snapable_CSPtr grid, double distanceToSnap);


        virtual void setGridSnappable(bool gridSnappable);
        virtual bool isGridSnappable() const;

    public slots:
        void on_mouseMoveEvent(const MouseMoveEvent& event);
        void on_mouseRelease_Event(const MouseReleaseEvent& event);

    private:

        std::shared_ptr<lc::SelectionManager> _selectionmanager;

        // Grid is snapable
        lc::Snapable_CSPtr _grid;

        // TRE when snapping to grid is evaluated
        bool _gridSnappable;

        // List of entities that are potential for snapping
        std::vector<lc::Snapable_CSPtr > _snapableEntities;

        // List of additional points a user can pick, to be implemented
        std::vector<lc::geo::Coordinate> _smartCoordinates;

        // What the minimal distance is when we send a snap event
        double _distanceToSnap;

        // Last Snap Point Event
        SnapPointEvent _lastSnapEvent;

        // List of entities 'under' the cursor
        std::vector<lc::EntityDistance> _entities;
};

// SNAPMANAGERIMPL_H
