#pragma once
#include <QWidget>
#include "lcadpaperviewer.h"

#include <QDialog>

//This class handles the paper viewers
namespace lc {
    namespace ui {
    	class LCADPaperViewerImpl: public QObject{
            Q_OBJECT
    	public:
    		LCADPaperViewerImpl(QWidget* parent);
    		void setDocument(std::shared_ptr<lc::storage::Document> document);
    		LCADPaperViewer* getViewer();
    		std::shared_ptr<lc::storage::Document> document(){return _document;};
    		storage::StorageManager_SPtr storageManager(){return _storageManager;};
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _viewers[_activeView]->snapManager();};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _viewers[_activeView]->cursor();};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _viewers[_activeView]->tempEntities();};
            viewer::manager::DragManager_SPtr dragManager(){return _viewers[_activeView]->dragManager();};
            storage::UndoManagerImpl_SPtr undoManager(){return _viewers[_activeView]->undoManager();};
            QPoint mapToGlobal(const QPoint& P){return _viewers[_activeView]->mapToGlobal(P);};
            lc::ui::LCADPaperViewer* viewer() {return _viewers[_activeView];};
            int getActiveView(){return _activeView;};
    	private:
    		std::shared_ptr<lc::storage::Document> _document;
    		std::vector<LCADPaperViewer*> _viewers;//Store all viewers
            storage::StorageManager_SPtr _storageManager;
            int _activeView = 0;
            QWidget* _parent;
        signals:
            void setActiveView(LCADViewer*,bool);
        public slots:
            void setActive(int);
    	};
    }
}