#include "lcadModelViewer.h"

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
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _viewers[0]->snapManager();};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _viewers[0]->cursor();};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _viewers[0]->tempEntities();};
            viewer::manager::DragManager_SPtr dragManager(){return _viewers[0]->dragManager();};
            storage::UndoManagerImpl_SPtr undoManager(){return _viewers[0]->undoManager();};
    	private:
    		QWidget* _parent;
    		std::shared_ptr<lc::storage::Document> _document;
    		std::vector<LCADModelViewer*> _viewers;//Store all viewers
            storage::StorageManager_SPtr _storageManager;
    	};
    }
}