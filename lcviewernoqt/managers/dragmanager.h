#pragma once

#include "cad/interface/draggable.h"
#include <cad/vo/entitycoordinate.h>
#include "../documentcanvas.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/dochelpers/entitycontainer.h"
#include <cad/base/cadentity.h>
#include "../drawables/lccursor.h"
#include "../drawables/tempentities.h"
#include <cad/document/document.h>
#include "cad/operations/builder.h"
#include "../events/dragpointsevent.h"
#include <nano-signal-slot/nano_signal_slot.hpp>

namespace LCViewer {
	class DragManager {
		friend class lc::Document;
		public:
			DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<Cursor> cursor, TempEntities_SPtr tempEntities, unsigned int size);

			void onMouseMove();
			void onMousePress();
			void onMouseRelease();

			bool entityDragged();

			Nano::Signal<void(const DragPointsEvent&)>& dragPointsEvent();
		private:
			unsigned int _size;
			lc::geo::Area _toleranceArea;

			std::vector<lc::geo::Coordinate> closeEntitiesDragPoints();
			std::vector<lc::geo::Coordinate> selectedEntitiesDragPoints();
			void moveEntities();

			DocumentCanvas_SPtr _docCanvas;
			std::shared_ptr<Cursor> _cursor;
			TempEntities_SPtr  _tempEntities;

			bool _entityDragged;

			lc::operation::Builder_SPtr _builder;

			lc::geo::Coordinate _selectedPoint;
			lc::EntityContainer<lc::entity::CADEntity_CSPtr> _selectedEntities;

			Nano::Signal<void(const DragPointsEvent&)> _dragPointsEvent;
	};
	using DragManager_SPtr = std::shared_ptr<DragManager>;
}