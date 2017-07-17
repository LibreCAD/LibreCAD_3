#include "lcvinsert.h"

using namespace LCViewer;

LCVInsert::LCVInsert(lc::entity::Insert_CSPtr insert) :
        LCVDrawItem(insert, true),
        _insert(insert) {

    _offset = _insert->position() - _insert->displayBlock()->base();

    for(auto entity : _insert->document()->entitiesByBlock(_insert->displayBlock()).asVector()) {
        append(entity);
    }

    _insert->document()->addEntityEvent().connect<LCVInsert, &LCVInsert::on_addEntityEvent>(this);
    _insert->document()->removeEntityEvent().connect<LCVInsert, &LCVInsert::on_removeEntityEvent>(this);
}

LCVInsert::~LCVInsert() {
    _insert->document()->addEntityEvent().disconnect<LCVInsert, &LCVInsert::on_addEntityEvent>(this);
    _insert->document()->removeEntityEvent().disconnect<LCVInsert, &LCVInsert::on_removeEntityEvent>(this);
}

void LCVInsert::append(lc::entity::CADEntity_CSPtr entity) {
    auto drawable = DocumentCanvas::asDrawable(entity->move(_offset));

    if(drawable == nullptr) {
        return;
    }

    _entities.insert(std::make_pair(entity->id(), drawable));
}

void LCVInsert::draw(LCViewer::LcPainter& _painter, const LCViewer::LcDrawOptions& options,
                               const lc::geo::Area& updateRect) const {
    for(auto entity : _entities) {
        entity.second->draw(_painter, options, updateRect);
    }
}

void LCVInsert::draw(DocumentCanvas_SPtr docCanvas) const {
    auto shared = _insert->shared_from_this();

    for(auto entity : _entities) {
        docCanvas->drawEntity(entity.second, shared);
    }
}

void LCVInsert::on_addEntityEvent(const lc::AddEntityEvent& event) {
    auto entity = event.entity();
    _entities.erase(entity->id());

    if(entity->block() == _insert->displayBlock()) {
        append(entity);
    }
}

void LCVInsert::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    auto entity = event.entity();

    if(!entity->block() || entity->id() == _insert->id()) {
        return;
    }

    _entities.erase(entity->id());
}

void LCVInsert::selected(bool selected) {
    LCVDrawItem::selected(selected);

    for(auto entity : _entities) {
        entity.second->selected(selected);
    }
}

lc::entity::CADEntity_CSPtr LCVInsert::entity() const {
    return _insert;
}
