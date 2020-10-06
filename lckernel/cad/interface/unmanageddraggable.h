#pragma once

#include <cad/storage/document.h>
#include <cad/operations/builder.h>
#include "draggable.h"

namespace lc {
namespace entity {
/**
 * @brief Draggable that has to remove and add itself from the document
 */
class UnmanagedDraggable {
public:
    /**
     * @brief Function called when a drag point is moved
     * @param position new position of the point
     */
    virtual void setDragPoint(lc::geo::Coordinate position) const = 0;

    /**
     * @brief Function called when a drag point of the entity is clicked
     * @param builder Builder to use to remove entities from the document, and keep one undo cycle
     * @param point Point clicked
     */
    virtual void onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const = 0;

    /**
     * @brief Function called when a clicked drag point is released
     * @param builder Builder to use to add entities from the document, and keep one undo cycle
     */
    virtual void onDragPointRelease(lc::operation::Builder_SPtr builder) const = 0;
};
}
}
