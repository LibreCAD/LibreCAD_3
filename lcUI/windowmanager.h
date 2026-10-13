#pragma once
#include "lcui_api.h"

#include <vector>

#include "cad/geometry/geocoordinate.h"
#include "mainwindow.h"

Q_DECLARE_METATYPE(lc::geo::Coordinate);

namespace lc
{
    namespace ui
    {
        class LC_UI_API WindowManager
        {
        public:
            /**
            * \brief Add initial MainWindow to mainWindows
            */
            static void init();

            /**
            * \brief Closes existing window and creates a new MainWindow
            */
            static void newFile(MainWindow* prevWindow);

            /**
            * \brief Opens a new MainWindow
            */
            static void openFile();

            /**
            * \brief Remove window from window manager
            */
            static void removeWindow(MainWindow* mainWindow);

            static std::vector<MainWindow*>& getMainWindows();

        public:
            static std::vector<MainWindow*> mainWindows;
        };
    }
}
