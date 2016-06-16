#pragma once

#include <cad/base/cadentity.h>
#include <cad/dochelpers/entitycontainer.h>
#include "../drawitems/lcvdrawitem.h"
#include "../events/drawevent.h"
#include "../documentcanvas.h"

namespace LCViewer {
	class TempEntities {
		public:
			TempEntities(DocumentCanvas_SPtr docCanvas);

			void addEntity(lc::entity::CADEntity_CSPtr entity);
			void removeEntity(lc::entity::CADEntity_CSPtr entity);

			void onDraw(DrawEvent const &event);

		private:
			DocumentCanvas_SPtr _docCanvas;
			lc::EntityContainer<lc::entity::CADEntity_CSPtr> _entities;
	};

	using TempEntities_SPtr = std::shared_ptr<TempEntities>;
}