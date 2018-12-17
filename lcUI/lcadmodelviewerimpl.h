#pragma once
#include "lcadmodelviewer.h"
#include <QDialog>

//This class handles the model viewers, ie. encapsulates codes for easy UI
// @TODO:
// Now it acts as connecting layer but later it should be QFrame consisting of multiple model views seperated by splitter
//Two consecutive models for now
namespace lc {
    namespace ui {
    	class LCADModelViewerImpl: public QWidget{
            Q_OBJECT
    	public:
    		LCADModelViewerImpl(QWidget* parent);
    		void setDocument(std::shared_ptr<lc::storage::Document> document);
    		LCADModelViewer* getViewer();
    		std::shared_ptr<lc::storage::Document> document(){return _document;};
    		storage::StorageManager_SPtr storageManager(){return _storageManager;};
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _viewers[_activeView]->snapManager();};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _viewers[_activeView]->cursor();};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _viewers[_activeView]->tempEntities();};
            viewer::manager::DragManager_SPtr dragManager(){return _viewers[_activeView]->dragManager();};
            storage::UndoManagerImpl_SPtr undoManager(){return _viewers[_activeView]->undoManager();};
            QPoint mapToGlobal(const QPoint& P){return _viewers[_activeView]->mapToGlobal(P);};
            lc::ui::LCADModelViewer* viewer() {return _viewers[_activeView];};
            int getActiveView(){return _activeView;};
    	private:
    		std::shared_ptr<lc::storage::Document> _document;
    		std::vector<LCADModelViewer*> _viewers;//Store all viewers
            storage::StorageManager_SPtr _storageManager;
            int _activeView = 0;
        signals:
            void setActiveView(LCADViewer*,bool);
        public slots:
            void setActive(int);
    	};
    }
}