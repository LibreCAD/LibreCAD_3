#include "lcvdrawitem.h"

using namespace lc::viewer;

LCVDrawItem::LCVDrawItem(const lc::entity::CADEntity_CSPtr& entity, bool selectable) :
        _selectable(selectable),
        _selected(false) {

}

bool LCVDrawItem::selectable() const {
    return _selectable;
}

bool LCVDrawItem::selected() const {
    return _selected;
}

void LCVDrawItem::selected(bool selected) {
    _selected = selected;
}
