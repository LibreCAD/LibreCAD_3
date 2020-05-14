#include "windowmanager.h"

using namespace lc::ui;

std::vector<MainWindow*> WindowManager::mainWindows;

void WindowManager::init()
{
    MainWindow* window = new MainWindow();
    window->showMaximized();
    mainWindows.push_back(window);
}

void WindowManager::newFile(MainWindow* prevWindow)
{
    for (auto iter=mainWindows.begin();iter!=mainWindows.end();++iter)
    {
        if (*iter == prevWindow)
        {
            (*iter)->close();
            iter = mainWindows.erase(iter);
            break;
        }
    }

    MainWindow* window = new MainWindow();
    window->showMaximized();
    mainWindows.push_back(window);
}

void WindowManager::openFile()
{
    MainWindow* window = new MainWindow();
    window->getCadMdiChild()->openFile();
    window->showMaximized();
    mainWindows.push_back(window);
}
