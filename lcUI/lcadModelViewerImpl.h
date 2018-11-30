#include "lcadModelViewer.h"

#include "cad/storage/storagemanagerimpl.h"
#include <drawables/gradientbackground.h>
#include <drawables/grid.h>
#include <managers/snapmanagerimpl.h>
#include <drawables/lccursor.h>
#include <drawables/tempentities.h>
#include <drawables/dragpoints.h>
#include "cad/storage/undomanagerimpl.h"

//This class handles the model viewers, ie. encapsulates codes for easy UI
namespace lc {
    namespace ui {
    	class LCADModelViewerImpl{
    	public:
    		LCADModelViewerImpl(QWidget* parent);
    		void setDocument(std::shared_ptr<lc::storage::Document> document);
    		LCADViewer* getViewer();
    		std::shared_ptr<lc::storage::Document> document(){return _document;};
    		storage::StorageManager_SPtr storageManager(){return _storageManager;};
    		std::shared_ptr<lc::viewer::drawable::Grid> grid(){return _grid;};
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _snapManager;};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _cursor;};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _tempEntities;};
            viewer::manager::DragManager_SPtr dragManager(){return _dragManager;};
            storage::UndoManagerImpl_SPtr undoManager(){return _undoManager;};
    	private:
    		QWidget* _parent;
    		std::shared_ptr<lc::storage::Document> _document;
    		std::vector<LCADViewer*> _viewers;//Store all viewers
            storage::StorageManager_SPtr _storageManager;

            std::shared_ptr<lc::viewer::drawable::Grid> _grid;
            std::shared_ptr<lc::viewer::drawable::GradientBackground> _gradientBackground;

            viewer::manager::SnapManagerImpl_SPtr _snapManager;
            std::shared_ptr<lc::viewer::drawable::Cursor> _cursor;

            viewer::drawable::TempEntities_SPtr _tempEntities;

            viewer::manager::DragManager_SPtr _dragManager;
            viewer::drawable::DragPoints_SPtr _dragPoints;

            storage::UndoManagerImpl_SPtr _undoManager;
    	};
    }
}