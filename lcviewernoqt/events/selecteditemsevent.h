#pragma once

#include "cad/base/cadentity.h"

/**
  * Event that get's dispatched each time the mouse is moved.
  */

namespace LCViewer {
class SelectedItemsEvent  {
    public:
        SelectedItemsEvent() {
        }
        SelectedItemsEvent(std::vector<lc::entity::CADEntity_CSPtr > entities) : _entities(entities) {
        }

        std::vector<lc::entity::CADEntity_CSPtr > entities() const {
            return _entities;
        }

    private:
        const std::vector<lc::entity::CADEntity_CSPtr > _entities;
};
}
