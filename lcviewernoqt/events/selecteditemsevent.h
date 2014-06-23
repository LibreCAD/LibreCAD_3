#ifndef SELECTEDITEMSEVENT_H
#define SELECTEDITEMSEVENT_H

#include "cad/base/cadentity.h"

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class SelectedItemsEvent  {
    public:
        SelectedItemsEvent() {
        }
        SelectedItemsEvent(std::vector<lc::CADEntity_CSPtr > entities) : _entities(entities) {
        }

        std::vector<lc::CADEntity_CSPtr > entities() const {
            return _entities;
        }

    private:
        const std::vector<lc::CADEntity_CSPtr > _entities;
};

#endif // SELECTEDITEMSEVENT_H
