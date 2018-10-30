#pragma once
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <lcadviewer.h>
#include "cad/meta/color.h"
#include <cad/storage/storagemanager.h>

#include "cad/storage/document.h"
#include "cad/storage/undomanager.h"
#include <drawables/lccursor.h>
#include <managers/snapmanager.h>
#include <drawables/gradientbackground.h>
#include <drawables/grid.h>
#include <drawables/dragpoints.h>
#include <drawables/tempentities.h>

#include <managers/snapmanagerimpl.h>
#include "cad/storage/undomanagerimpl.h"

#include <file.h>
#include <managers/metainfomanager.h>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
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
                void setDestroyCallback(kaguya::LuaRef destroyCallback);

                void keyPressEvent(QKeyEvent* event);

            public slots:

                void ctxMenu(const QPoint& pos);

            signals:

                void keyPressed(QKeyEvent* event);

            public:
                QWidget* view() const;

                std::shared_ptr<lc::storage::Document> document() const;

                lc::storage::UndoManager_SPtr undoManager() const;

                lc::viewer::manager::SnapManager_SPtr snapManager() const;

                lc::storage::StorageManager_SPtr storageManager() const;

                lc::ui::LCADViewer* viewer() const { return _viewer; }

                std::shared_ptr<lc::viewer::drawable::Cursor> cursor() const;

                /**
                 * @brief Get the selected layer
                 * @return Selected layer
                 */
                lc::meta::Layer_CSPtr activeLayer() const;

                void setActiveLayer(const lc::meta::Layer_CSPtr& activeLayer);

                /**
                 * @brief Get the MetaInfo manager
                 * @return MetaInfoManager
                 */
                lc::ui::MetaInfoManager_SPtr metaInfoManager() const;

                /**
                 * \brief Get container of temporary entities
                 * \return Temporary entities container
                 */
                lc::viewer::drawable::TempEntities_SPtr tempEntities();

                /**
                 * \brief Get selected entities
                 * \return Selected entities
                 * Return a vector of selected entities.
                 * This function was added for Lua which can't access EntityContainer functions
                 */
                std::vector<lc::entity::CADEntity_CSPtr> selection();

                void saveFile();

                /**
                 * \brief Get window ID
                 * \return Window ID
                 * This is used by Lua to distinguish the different windows.
                 */
                unsigned int id() const;

                /**
                 * \brief Set window ID
                 * \param id Window ID
                 * This is used by Lua to distinguish the different windows.
                 * This function should not be used.
                 */
                void setId(unsigned int id);

                const viewer::manager::SnapManagerImpl_SPtr& getSnapManager() const;

            private:
                unsigned int _id;

                kaguya::LuaRef _destroyCallback;

                std::shared_ptr<lc::storage::Document> _document;
                storage::UndoManagerImpl_SPtr _undoManager;

                std::shared_ptr<lc::viewer::drawable::Grid> _grid;
                std::shared_ptr<lc::viewer::drawable::GradientBackground> _gradientBackground;
                std::shared_ptr<lc::viewer::drawable::Cursor> _cursor;
                viewer::manager::SnapManagerImpl_SPtr _snapManager;

                viewer::manager::DragManager_SPtr _dragManager;
                viewer::drawable::DragPoints_SPtr _dragPoints;
                storage::StorageManager_SPtr _storageManager;
                viewer::drawable::TempEntities_SPtr _tempEntities;

                meta::Layer_CSPtr _activeLayer;
                ui::MetaInfoManager_SPtr _metaInfoManager;

                QScrollBar* horizontalScrollBar;
                QScrollBar* verticalScrollBar;

                ui::LCADViewer* _viewer;
        };
    }
}
