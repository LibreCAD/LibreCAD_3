#include "tempentities.h"

using namespace lc;
using namespace lc::viewer;
using namespace lc::viewer::drawable;

TempEntities::TempEntities(DocumentCanvas_SPtr docCanvas) :
		_docCanvas(std::move(docCanvas)) {
}

void TempEntities::addEntity(lc::entity::CADEntity_CSPtr entity) {
    _entities.insert(std::move(entity));
    requestUpdateEvent()();
}

void TempEntities::removeEntity(lc::entity::CADEntity_CSPtr entity) {
	_entities.remove(std::move(entity));
    requestUpdateEvent()();
}

void TempEntities::onDraw(lc::viewer::event::DrawEvent const &event) {
    _entities.each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr entity) {
        _docCanvas->drawEntity(event.painter(), _docCanvas->asDrawable(entity));
	});
}
