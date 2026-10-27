#include "windowmanager.h"

#include <QObject>
#include <QMetaObject>
#include <memory>

#ifdef LC_WITH_PYTHONSCRIPT
#include "python/pyeventhooks.h"
#endif

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
    std::unique_ptr<MainWindow> window(new MainWindow());
#ifdef LC_WITH_PYTHONSCRIPT
    // File loading happens before this window is inserted into mainWindows.
    // Keep callback/event routing attached to its owner throughout dispatch.
    lc::ui::python::MainWindowExecutionScope ownerScope(window.get());
#endif
    if (!window->cadMdiChild()->openFile()) return;
    window->showMaximized();
    mainWindows.push_back(window.release());
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
