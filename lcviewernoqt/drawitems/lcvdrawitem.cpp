#include "lcvdrawitem.h"

using namespace lc::viewer;

LCVDrawItem::LCVDrawItem(const lc::entity::CADEntity_CSPtr& entity, bool selectable) :
        _selectable(selectable),
        _selected(false),
	_cacheable(true),
	_autostroke(true) {
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

bool LCVDrawItem::cacheable() const {
    return _cacheable;
}

void LCVDrawItem::cacheable(bool cache) {
    _cacheable = cache;
}

bool LCVDrawItem::autostroke() const {
    return _autostroke;
}

void LCVDrawItem::autostroke(bool stroke) {
    _autostroke = stroke;
}

