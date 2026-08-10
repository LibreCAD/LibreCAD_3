#include <QMainWindow>
#include <kaguya/kaguya.hpp>

#include <set>
#include <map>

namespace lc
{
namespace ui
{
/**
 * \brief Lua Operations Loader
 */
class OperationLoader
{
public:
    /**
     * \brief Constructor for OperationLoader
     * \param luaPath string path to lcUI folder
     * \param qmainWindow QMainWindow which will be casted to MainWindow
     * \param luaState lua state
     */
    OperationLoader(const std::string& luaPath, QMainWindow* qmainWindow, kaguya::State& luaState);

    /**
     * \brief Load operations from lua scripts and run init functions
     * \param luaPath string path to lcUI folder
     */
    void loadLuaOperations(const std::string& luaPath);

    /**
     * \brief Initialize each operation on the basis of found properties
     * \param vkey - Operation key for state table
     */
    void initializeOperation(const std::string& vkey);

    /**
     * \brief Add cli command for operation
     * \param vkey - Operation key for state table Eg. "LineOperations"
     * \param opkey - Key for operation property Eg. "command_line"
     */
    void addOperationCommandLine(const std::string& vkey, const std::string& opkey);

    /**
     * \brief Add menu action for operation
     */
    void addOperationMenuAction(const std::string& vkey, const std::string& opkey);

    /**
     * \brief Add toolbar icon for operation
     */
    void addOperationIcon(const std::string& vkey, const std::string& opkey);

    /**
     * \brief Add extra icons in Current operation group of toolbar when operation is run
     */
    void addOperationToolbarOptions(const std::string& vkey, const std::string& opkey);

    /**
     * \brief Add operation to context menu
     */
    void addContextMenuOperations(const std::string& vkey);

    /**
     * \brief Add context transitions for lua operations
     */
    void addContextTransitions(const std::string& vkey, const std::string& opkey);

#ifdef LC_WITH_PYTHONSCRIPT
    /**
     * \brief Phase 5 PR-5.2 — walk `lc.operation_registry` (a Python
     * dict populated by `@lc.register_operation`), and for each entry,
     * do the same wiring the Lua path does: CliCommand::addCommand,
     * Toolbar::addButton, MainWindow::connectMenuItem, addOperationOptions,
     * ContextMenuManager::addOperation/addTransition.  Also pushes a
     * Python-registry resolver onto MainWindow's ordered resolver list
     * (phase 4 PR-7) so `MainWindow::runOperationByName(name)` resolves
     * against BOTH sources — later-registered (Python) wins by design.
     *
     * Iteration is SORTED BY NAME (Lua's std::set gives alphabetical
     * order for toolbar/menu wiring; match it so ui_settings.json's
     * toolbar-layout persistence stays stable).
     *
     * Name collisions with already-registered Lua vkeys are rejected
     * with a logged warning (registry share a namespace with toolbar
     * labels + ContextMenuManager _operationMap + ui_settings.json).
     */
    void loadPythonOperations();
#endif

private:
    /**
     * \brief Load lua folder eg, createActions
     * \param folderName - name of the folder
     * \param fileToSkip - skip this file (createOperations.lua in createActions, operations.lua in actions)
     * \param luaPath string path to lcUI folder
     */
    void loadLuaFolder(const std::string folderName, const std::string& fileToSkip, const std::string& luaPath);

    /**
     * \brief Determine which group each operation is in and store in groupElements
     */
    void getSetOfGroupElements();

private:
    kaguya::State& _L;
    QMainWindow* qmainWindow;

    std::map<std::string, std::set<std::string>> groupElements;
    std::map<std::string, std::set<std::string>> foundProperties;
    std::map<std::string, std::string> groupNames;
};
}
}
