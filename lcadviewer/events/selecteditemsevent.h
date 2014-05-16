#ifndef SELECTEDITEMSEVENT_H
#define SELECTEDITEMSEVENT_H

#include <QList>
#include "cad/base/cadentity.h"

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class SelectedItemsEvent  {
    public:
        SelectedItemsEvent() {
        }
        SelectedItemsEvent(QList<lc::CADEntity_CSPtr > entities) : _entities(entities) {
        }

        QList<lc::CADEntity_CSPtr > entities() const {
            return _entities;
        }

    private:
        QList<lc::CADEntity_CSPtr > _entities;
};

#endif // SELECTEDITEMSEVENT_H
