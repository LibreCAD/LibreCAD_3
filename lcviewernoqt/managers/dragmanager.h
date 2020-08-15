#pragma once

#include "cad/interface/draggable.h"
#include <cad/vo/entitycoordinate.h>
#include "../documentcanvas.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/storage/entitycontainer.h"
#include <cad/base/cadentity.h>
#include "../drawables/lccursor.h"
#include "../drawables/tempentities.h"
#include <cad/storage/document.h>
#include "cad/operations/entitybuilder.h"
#include "../events/dragpointsevent.h"
#include <nano-signal-slot/nano_signal_slot.hpp>
#include <cad/operations/builder.h>

namespace lc {
namespace viewer {
namespace manager {
/**
 * \brief Checks if drag points are near the cursor and allow to modify the entities.
 */
class DragManager {
public:
    /**
     * \brief Create manager
     * \param docCanvas Document canvas containing the entities to drag
     * \param cursor Pointer to cursor
     * \param tempEntities Pointer to temp entity container
     * \param size Box size around the points
     */
    DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<drawable::Cursor> cursor,
                drawable::TempEntities_SPtr tempEntities, unsigned int size);

    /**
     * \brief Called each time the mouse move.
     * If no point is selected, it searches for entities near the cursor.
     * If a point is selected, it modifies the selected entity
     */
    void onMouseMove();

    /**
     * \brief Called each time the mouse is clicked.
     * Check if a point is under the cursor.
     */
    void onMousePress();

    /**
     * \brief Called each time the mouse is released.
     * Update the entities in the document.
     */
    void onMouseRelease();

    /**
     * \brief Return true if a point is selected.
     * This is used to disable selection.
     */
    bool entityDragged();

    Nano::Signal<void(const lc::viewer::event::DragPointsEvent&)>& dragPointsEvent();

private:
    unsigned int _size;
    lc::geo::Area _toleranceArea;

    std::vector<lc::geo::Coordinate> selectedEntitiesDragPoints();

    void moveEntities();

    DocumentCanvas_SPtr _docCanvas;
    std::shared_ptr<drawable::Cursor> _cursor;
    drawable::TempEntities_SPtr _tempEntities;

    bool _entityDragged;

    lc::operation::Builder_SPtr _builder;
    lc::operation::EntityBuilder_SPtr _entityBuilder;

    lc::geo::Coordinate _selectedPoint;
    std::vector<lc::entity::CADEntity_CSPtr> _replacementEntities;

    Nano::Signal<void(const lc::viewer::event::DragPointsEvent&)> _dragPointsEvent;
};

DECLARE_SHORT_SHARED_PTR(DragManager)
}
}
}
