#include "tempentities.h"

using namespace LCViewer;

TempEntities::TempEntities(DocumentCanvas_SPtr docCanvas) : _docCanvas(docCanvas) {
}

void TempEntities::addEntity(lc::entity::CADEntity_CSPtr entity) {
	auto drawable = _docCanvas->asDrawable(entity);

	_entities.insert(std::dynamic_pointer_cast<const lc::entity::CADEntity>(drawable));
}

void TempEntities::removeEntity(lc::entity::CADEntity_CSPtr entity) {
	_entities.remove(entity);
}

void TempEntities::onDraw(DrawEvent const &event) {
	_entities.each<const LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_CSPtr entity) {
		_docCanvas->drawEntity(entity);
	});
}