#pragma once

#include <cad/base/cadentity.h>
#include <cad/storage/entitycontainer.h>
#include "../drawitems/lcvdrawitem.h"
#include "../events/drawevent.h"
#include "../documentcanvas.h"

namespace lc {
namespace viewer {
namespace drawable {
/**
 * \brief Storage for entities which needs to be displayed without being in the document.
 * This is useful when modifying an entity to display a preview.
 */
class TempEntities {
public:
    /**
     * \brief Create container
     * \param docCanvas Document canvas to draw on
     */
    TempEntities(DocumentCanvas_SPtr docCanvas);

    /**
     * \brief Add a new entity to the container
     * \param entity Entity to add
     */
    void addEntity(lc::entity::CADEntity_CSPtr entity);

    /**
     * \brief Remove entity from the container
     * \param entity Entity to remove
     */
    void removeEntity(lc::entity::CADEntity_CSPtr entity);

    /**
     * \brief Draw all the entities
     */
    void onDraw(event::DrawEvent const& event);
    virtual Nano::Signal<void()>& requestUpdateEvent() {
        return _requestUpdateEvent;
    };

private:
    DocumentCanvas_SPtr _docCanvas;
    lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr> _entities;
    Nano::Signal<void()> _requestUpdateEvent;
};

using TempEntities_SPtr = std::shared_ptr<TempEntities>;
}
}
}