#pragma once

#include "snapmanager.h"
#include "../documentcanvas.h"
#include "../events/LocationEvent.h"
#include <cad/interface/snapconstrain.h>

/*!
 * \brief Implements the SnapManager interface
 */

namespace lc {
    namespace viewer {
        namespace event {
            class MouseMoveEvent;
            class SnapPointEvent;
        }

        namespace manager {
            class SnapManagerImpl : public SnapManager {
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
                    SnapManagerImpl(DocumentCanvas_SPtr view, lc::entity::Snapable_CSPtr grid, double distanceToSnap);

                    virtual ~SnapManagerImpl() = default;

                    virtual void setGridSnappable(bool enabled);

                    virtual bool isGridSnappable() const;

                    virtual void setIntersectionsSnappable(bool enabled);

                    virtual bool isIntersectionsSnappable() const;

                    virtual void setMiddleSnappable(bool enabled);

                    virtual void setEntitySnappable(bool enabled);

                    virtual void setDeviceLocation(int x, int y);


                    lc::SimpleSnapConstrain snapConstrain() const;

                    void snapConstrain(const lc::SimpleSnapConstrain& snapConstrain);

                    virtual Nano::Signal<void(const event::SnapPointEvent&)>& snapPointEvents();

                private:

                    // Grid is snapable
                    lc::entity::Snapable_CSPtr _grid;

                    // TRUE when snapping to grid is evaluated
                    bool _gridSnappable;
                    // TRUE when snapping to intersections are enabled
                    bool _snapIntersections;

                    // List of entities that are potential for snapping
                    std::vector<lc::entity::Snapable_CSPtr> _snapableEntities;

                    // List of additional points a user can pick, to be implementedx
                    std::vector<lc::geo::Coordinate> _smartCoordinates;

                    // What the minimal distance is when we send a snap event
                    double _distanceToSnap;

                    // Last Snap Point Event
                    event::SnapPointEvent _lastSnapEvent;

                    // Snap Point Event
                    Nano::Signal<void(const lc::viewer::event::SnapPointEvent&)> _snapPointEvent;

                    DocumentCanvas_SPtr _view;

                    lc::SimpleSnapConstrain _snapConstrain;
            };


            DECLARE_SHORT_SHARED_PTR(SnapManagerImpl)
        }
    }
}
