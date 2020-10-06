#pragma once

#include <QMenu>
#include <cad/base/cadentity.h>
#include <kaguya/kaguya.hpp>

#include "mainwindow.h"
#include "widgets/guiAPI/menu.h"

namespace lc
{
namespace ui
{
/**
 * \brief Context Menu Manager
 */
class ContextMenuManager
{
public:
    /**
     * \brief Get Context Menu
     * \param MainWindow pointer
     * \return pointer to ContextMenuManager
     */
    static ContextMenuManager* GetContextMenuManager(lc::ui::MainWindow* mainWindowIn);

    /**
     * \brief Get Context Menu
     * \param int instance id
     * \return pointer to ContextMenuManager
     */
    static ContextMenuManager* GetContextMenuManager(int instanceIdIn);

    /**
     * \brief Get Instance Id of context for the given mainwindow
     * \param MainWindow pointer
     * \return int instance id
     */
    static int GetInstanceId(lc::ui::MainWindow* mainWindowIn);

    /**
     * \brief Generate menu for the selected entities / no entity selected
     * \param vector of selected entities cad entity shared pointers
     */
    void generateMenu(api::Menu* menu, std::vector<lc::entity::CADEntity_CSPtr> selectedEntities);

    /**
     * \brief Add operation to the list (callled by operation loader)
     * \param string Operation Name (key)
     * \param string group name the operation belongs to
     */
    void addOperation(const std::string& key, const std::string& groupName);

    /**
     * \brief Add commands for active state
     */
    void activeCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities);

    /**
     * \brief Add commands for selected state
     */
    void selectedCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities);

    /**
     * \brief Add commands for inactive state
     */
    void inactiveCommands(api::Menu* menu);

    /**
     * \brief Add commands for the current operation
     */
    void operationContextCommands(api::Menu* menu, const std::vector<lc::entity::CADEntity_CSPtr>& selectedEntities);

    /**
     * \brief Add transition to the transitions map
     */
    void addTransition(std::string entityName, std::string fname, std::vector<std::string> transitionList);

private:
    std::string cleanOperationName(const std::string& opName) const;

    std::string cleanTransitionName(const std::string& transitionName) const;

    /**
     * \brief Constructor for ContextMenuManager
     * \param MainWindow pointer
     */
    ContextMenuManager(lc::ui::MainWindow* mainWindowIn);

    void addSnapCommands(api::Menu* menu);

private:
    static std::map<lc::ui::MainWindow*, int> instancesId;
    static std::map<int, ContextMenuManager*> instances;
    kaguya::State _L;
    std::map<std::string, std::vector<std::string>> _operationMap;
    lc::ui::MainWindow* _mainWindow;
    static int _instanceCount;
    std::map<std::string, std::map<std::string, std::vector<std::string>>> _transitionMap;
};
}
}
