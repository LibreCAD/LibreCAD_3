#pragma once

#include <vector>
#include "mainwindow.h"

namespace lc
{
    namespace ui
    {
        class WindowManager
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

            static std::vector<MainWindow*>& getMainWindows();

        public:
            static std::vector<MainWindow*> mainWindows;
        };
    }
}
