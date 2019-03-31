#pragma once
#include "lcadviewer.h"

#include "cad/storage/storagemanagerimpl.h"
#include <drawables/gradientbackground.h>
#include <drawables/grid.h>
#include <managers/snapmanagerimpl.h>
#include <drawables/lccursor.h>
#include <drawables/tempentities.h>
#include <drawables/dragpoints.h>
#include "cad/storage/undomanagerimpl.h"

namespace lc {
    namespace ui {
        class LCADModelViewer : public LCADViewer {
            Q_OBJECT
        public:
            LCADModelViewer(QWidget* parent,int id);
            void setDocument(std::shared_ptr<lc::storage::Document> document);
            viewer::manager::SnapManagerImpl_SPtr snapManager(){return _snapManager;};
            std::shared_ptr<lc::viewer::drawable::Cursor> cursor(){return _cursor;};                    
            viewer::drawable::TempEntities_SPtr tempEntities(){return _tempEntities;};
            viewer::manager::DragManager_SPtr dragManager(){return _dragManager;};
            storage::UndoManagerImpl_SPtr undoManager(){return _undoManager;};
            void setFocused(bool v){_isActive=v;};
        signals:
            void setActive(int);
        private:
            std::shared_ptr<lc::viewer::drawable::GradientBackground> _gradientBackground;
            std::shared_ptr<lc::viewer::drawable::Grid> _grid;
            viewer::manager::SnapManagerImpl_SPtr _snapManager;
            std::shared_ptr<lc::viewer::drawable::Cursor> _cursor;
            viewer::drawable::TempEntities_SPtr _tempEntities;
            viewer::manager::DragManager_SPtr _dragManager;
            viewer::drawable::DragPoints_SPtr _dragPoints;
            storage::UndoManagerImpl_SPtr _undoManager;
            int _id;
            bool _isActive=false;//is Active
        private slots:
            void onMouseMoveEvent();
        };
    }
}