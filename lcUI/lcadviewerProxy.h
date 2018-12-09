#pragma once
#include <QWidget>
#include <lcadModelViewerImpl.h>
#include <lcadPaperViewerImpl.h>
#include "cad/storage/document.h"
#include <cad/storage/storagemanager.h>


//This class manages the model impl and paper impl
namespace lc {
    namespace ui {
    	class LCADViewerProxy: public QWidget{
            Q_OBJECT
    	public:
    		LCADViewerProxy(QWidget* parent);
    		void setDocument(std::shared_ptr<lc::storage::Document> document);
    		std::shared_ptr<lc::storage::Document> document(){return _modelViewerImpl->document();};
    		storage::StorageManager_SPtr storageManager(){return _modelViewerImpl->storageManager();};
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _modelViewerImpl->snapManager();};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _modelViewerImpl->cursor();};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _modelViewerImpl->tempEntities();};
//            viewer::manager::DragManager_SPtr dragManager(){return _viewers[_activeView]->dragManager();};
            storage::UndoManagerImpl_SPtr undoManager(){return _modelViewerImpl->undoManager();};
            QPoint mapToGlobal(const QPoint& P){return _activeView->mapToGlobal(P);};
            lc::ui::LCADViewer* viewer() {return _activeView;};
    	private:
            LCADViewer* _activeView;
        	LCADModelViewerImpl* _modelViewerImpl;
        	LCADPaperViewerImpl* _paperViewers;
       	signals:
            void mouseMoveEvent();
            void mousePressEvent();
            void mouseReleaseEvent();
            void keyPress(int);
        public slots:
            void setActive(LCADViewer* view);
    	};
    }
}