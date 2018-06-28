#include "lcvdrawitem.h"
using namespace LCViewer;

LCVDrawItem::LCVDrawItem(const lc::entity::CADEntity_CSPtr& entity, bool selectable) :
        CADEntity(entity, true),
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

lc::entity::CADEntity_CSPtr LCVDrawItem::move(const lc::geo::Coordinate& offset) const {
    return entity()->move(offset);
}

lc::entity::CADEntity_CSPtr LCVDrawItem::copy(const lc::geo::Coordinate& offset) const {
    return entity()->copy(offset);
}

lc::entity::CADEntity_CSPtr LCVDrawItem::rotate(const lc::geo::Coordinate& rotation_center, const double rotation_angle) const {
    return entity()->rotate(rotation_center, rotation_angle);
}

lc::entity::CADEntity_CSPtr LCVDrawItem::scale(const lc::geo::Coordinate& scale_center, const lc::geo::Coordinate& scale_factor) const {
    return entity()->scale(scale_center, scale_factor);
}

lc::entity::CADEntity_CSPtr LCVDrawItem::mirror(const lc::geo::Coordinate& axis1, const lc::geo::Coordinate& axis2) const {
    return entity()->mirror(axis1, axis2);
}

const lc::geo::Area LCVDrawItem::boundingBox() const {
    return entity()->boundingBox();
}

lc::entity::CADEntity_CSPtr
LCVDrawItem::modify(lc::Layer_CSPtr layer, const lc::MetaInfo_CSPtr metaInfo, lc::Block_CSPtr block) const {
    return entity()->modify(layer, metaInfo, lc::Block_CSPtr());
}

void LCVDrawItem::dispatch(lc::EntityDispatch& dispatch) const {
    entity()->dispatch(dispatch);
}