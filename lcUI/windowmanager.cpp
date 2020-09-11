#include "windowmanager.h"

#include <QObject>
#include <QMetaObject>

using namespace lc::ui;

std::vector<MainWindow*> WindowManager::mainWindows;

void WindowManager::init()
{
    qRegisterMetaType<lc::geo::Coordinate>();
    MainWindow* window = new MainWindow();
    window->showMaximized();
    mainWindows.push_back(window);
}

void WindowManager::newFile(MainWindow* prevWindow)
{
    MainWindow* window = new MainWindow();
    window->showMaximized();
    mainWindows.push_back(window);
}

void WindowManager::openFile()
{
    MainWindow* window = new MainWindow();
    window->cadMdiChild()->openFile();
    window->showMaximized();
    mainWindows.push_back(window);
}

void WindowManager::removeWindow(MainWindow* mainWindow) 
{
    for (auto iter = mainWindows.begin(); iter != mainWindows.end(); ++iter)
    {
        if (*iter == mainWindow)
        {
            iter = mainWindows.erase(iter);
            break;
        }
    }
}

std::vector<MainWindow*>& WindowManager::getMainWindows()
{
    return mainWindows;
}
