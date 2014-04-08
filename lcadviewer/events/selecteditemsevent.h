#ifndef SELECTEDITEMSEVENT_H
#define SELECTEDITEMSEVENT_H

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class SelectedItemsEvent  {
    public:
        SelectedItemsEvent() {
        }
        SelectedItemsEvent(QList<shared_ptr<const lc::CADEntity> > entities) : _entities(entities) {
        }

        QList<shared_ptr<const lc::CADEntity> > entities() const {
            return _entities;
        }

    private:
        QList<shared_ptr<const lc::CADEntity> > _entities;
};

#endif // SELECTEDITEMSEVENT_H
