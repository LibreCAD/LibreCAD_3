#include "lcadModelViewer.h"

//This class handles the model viewers, ie. encapsulates codes for easy UI
// @TODO:
// Now it acts as connecting layer but later it should be QFrame consisting of multiple model views seperated by splitter
namespace lc {
    namespace ui {
    	class LCADModelViewerImpl{
    	public:
    		LCADModelViewerImpl(QWidget* parent);
    		void setDocument(std::shared_ptr<lc::storage::Document> document);
    		LCADViewer* getViewer();
    		std::shared_ptr<lc::storage::Document> document(){return _document;};
    		storage::StorageManager_SPtr storageManager(){return _storageManager;};
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _viewers[_activeView]->snapManager();};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _viewers[_activeView]->cursor();};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _viewers[_activeView]->tempEntities();};
            viewer::manager::DragManager_SPtr dragManager(){return _viewers[_activeView]->dragManager();};
            storage::UndoManagerImpl_SPtr undoManager(){return _viewers[_activeView]->undoManager();};
            void setActiveView(int x){_activeView=x;};
            int getActiveView(){return _activeView;};
    	private:
    		QWidget* _parent;
    		std::shared_ptr<lc::storage::Document> _document;
    		std::vector<LCADModelViewer*> _viewers;//Store all viewers
            storage::StorageManager_SPtr _storageManager;
            int _activeView = 0;
    	};
    }
}