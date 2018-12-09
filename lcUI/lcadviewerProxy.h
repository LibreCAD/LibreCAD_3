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
    		std::shared_ptr<lc::storage::Document> document(){return _isModel?_modelViewerImpl->document():_paperViewers->document();};
    		storage::StorageManager_SPtr storageManager(){return _isModel?_modelViewerImpl->storageManager():_paperViewers->storageManager();};
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _isModel?_modelViewerImpl->snapManager():_paperViewers->snapManager();};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _isModel?_modelViewerImpl->cursor():_paperViewers->cursor();};
            viewer::drawable::TempEntities_SPtr tempEntities(){return _isModel?_modelViewerImpl->tempEntities():_paperViewers->tempEntities();};
//            viewer::manager::DragManager_SPtr dragManager(){return _viewers[_activeView]->dragManager();};
            storage::UndoManagerImpl_SPtr undoManager(){return _isModel?_modelViewerImpl->undoManager():_paperViewers->undoManager();};
            QPoint mapToGlobal(const QPoint& P){return _activeView->mapToGlobal(P);};
            lc::ui::LCADViewer* viewer() {return _activeView;};
    	private:
            LCADViewer* _activeView;
        	LCADModelViewerImpl* _modelViewerImpl;
        	LCADPaperViewerImpl* _paperViewers;
        	bool _isModel;//If selected viewer is model viewer
       	signals:
            void mouseMoveEvent();
            void mousePressEvent();
            void mouseReleaseEvent();
            void keyPress(int);
        public slots:
            void setActive(LCADViewer* view,bool isModel);
    	};
    }
}