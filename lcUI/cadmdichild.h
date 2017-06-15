#pragma once
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <lcadviewer.h>
#include "cad/meta/color.h"
#include <cad/document/storagemanager.h>

#include "cad/document/document.h"
#include "cad/document/undomanager.h"
#include <drawables/lccursor.h>
#include <managers/snapmanager.h>
#include <drawables/gradientbackground.h>
#include <drawables/grid.h>
#include <drawables/dragpoints.h>
#include <drawables/tempentities.h>

#include <managers/snapmanagerimpl.h>
#include "cad/dochelpers/undomanagerimpl.h"

#include <file.h>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "lua-intf/LuaIntf/LuaIntf.h"

class CadMdiChild : public QWidget {
        Q_OBJECT

    public:
        explicit CadMdiChild(QWidget* parent = 0);
        ~CadMdiChild();

		/**
		 * \brief Create a new document.
		 */
        void newDocument();

        /**
         * \brief Load existing file.
         * \param path Path to file
         * \return bool True if the file was correctly opened, false otherwise.
         */
        bool openFile();

        /**
         * \brief Give function to call when window is destroyed
         * \param callback Lua function
         */
        void setDestroyCallback(LuaIntf::LuaRef callback);

		void keyPressEvent(QKeyEvent* event);

    public slots:
        void ctxMenu(const QPoint& pos);

	signals:
		void keyPressed(QKeyEvent* event);

    public:
        QWidget* view() const;
        std::shared_ptr<lc::Document> document() const;
        lc::UndoManager_SPtr undoManager() const;
        LCViewer::SnapManager_SPtr  snapManager() const;
        lc::StorageManager_SPtr storageManager() const;
        LCViewer::LCADViewer* viewer() const {return _viewer;}
        std::shared_ptr<LCViewer::Cursor> cursor() const;

        /**
         * @brief Get the selected layer
         * @return Selected layer
         */
        lc::Layer_CSPtr activeLayer() const;

        void setActiveLayer(const lc::Layer_CSPtr& activeLayer);

        /**
         * \brief Get container of temporary entities
         * \return Temporary entities container
         */
		LCViewer::TempEntities_SPtr tempEntities();

		/**
		 * \brief Get selected entities
		 * \return Selected entities
		 * Return a vector of selected entities.
		 * This function was added for Lua which can't access EntityContainer functions
		 */
		std::vector<lc::entity::CADEntity_SPtr> selection();
        void saveFile();

        /**
         * \brief Get window ID
         * \return Window ID
         * This is used by Lua to distinguish the different windows.
         */
        unsigned int id();

        /**
         * \brief Set window ID
         * \param id Window ID
         * This is used by Lua to distinguish the different windows.
         * This function should not be used.
         */
        void setId(unsigned int id);

    private:
        unsigned int _id;

        LuaIntf::LuaRef _destroyCallback;

        std::shared_ptr<lc::Document> _document;
        lc::UndoManagerImpl_SPtr _undoManager;

        std::shared_ptr<LCViewer::Grid> _grid;
        std::shared_ptr<LCViewer::GradientBackground> _gradientBackground;
        std::shared_ptr<LCViewer::Cursor>  _cursor;
        LCViewer::SnapManagerImpl_SPtr  _snapManager;
        LCViewer::DragManager_SPtr _dragManager;
        LCViewer::DragPoints_SPtr _dragPoints;
        lc::StorageManager_SPtr _storageManager;
		LCViewer::TempEntities_SPtr _tempEntities;

		lc::Layer_CSPtr _activeLayer;

        QScrollBar* horizontalScrollBar;
        QScrollBar* verticalScrollBar;

        LCViewer::LCADViewer* _viewer;


};
