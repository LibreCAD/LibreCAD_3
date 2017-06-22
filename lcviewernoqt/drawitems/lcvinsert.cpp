#include "lcvinsert.h"
#include "../documentcanvas.h"

using namespace LCViewer;

LCVInsert::LCVInsert(lc::entity::Insert_CSPtr insert) :
        lc::entity::Insert(insert, true),
        LCVDrawItem(true) {

    _offset = position() - displayBlock()->base();

    for(auto entity : document()->entitiesByBlock(displayBlock()).asVector()) {
        append(entity);
    }

    document()->addEntityEvent().connect<LCVInsert, &LCVInsert::on_addEntityEvent>(this);
    document()->removeEntityEvent().connect<LCVInsert, &LCVInsert::on_removeEntityEvent>(this);
}

LCVInsert::~LCVInsert() {
    document()->addEntityEvent().disconnect<LCVInsert, &LCVInsert::on_addEntityEvent>(this);
    document()->removeEntityEvent().disconnect<LCVInsert, &LCVInsert::on_removeEntityEvent>(this);
}

void LCVInsert::append(lc::entity::CADEntity_CSPtr entity) {
    auto drawable = DocumentCanvas::asDrawable(entity->move(_offset));

    if(drawable == nullptr) {
        return;
    }

    _entities.insert(std::dynamic_pointer_cast<const lc::entity::CADEntity>(drawable));
}

void LCVInsert::draw(LCViewer::LcPainter& _painter, const LCViewer::LcDrawOptions& options,
                               const lc::geo::Area& updateRect) const {
    for(auto entity : _entities) {
        std::dynamic_pointer_cast<const LCVDrawItem>(entity)->draw(_painter, options, updateRect);
    }
}

void LCVInsert::draw(DocumentCanvas_SPtr docCanvas) const {
    for(auto entity : _entities) {
        docCanvas->drawEntity(std::dynamic_pointer_cast<const LCVDrawItem>(entity), shared_from_this());
    }
}

void LCVInsert::on_addEntityEvent(const lc::AddEntityEvent& event) {
    auto entity = event.entity();
    _entities.erase(entity);

    if(entity->block() == displayBlock()) {
        append(entity);
    }
}

void LCVInsert::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    auto entity = event.entity();

    if(!entity->block() || entity->id() == id()) {
        return;
    }

    _entities.erase(entity);
}