#include "operationloader.h"

#include "mainwindow.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"
#include "managers/contextmenumanager.h"

#include <QDir>
#include <QString>
#include <QStringList>

using namespace lc::ui;

OperationLoader::OperationLoader(const std::string& luaPath, QMainWindow* qmainWindow, kaguya::State& luaState) 
    :
    qmainWindow(qmainWindow),
    _L(luaState){
    loadLuaOperations(luaPath);
}

void OperationLoader::loadLuaOperations(const std::string& luaPath) {
    // call operations and createoperations first
    _L.dofile(luaPath + "/actions/operations.lua");
    _L.dofile(luaPath + "/createActions/createOperations.lua");
    loadLuaFolder("createActions", "createOperations", luaPath);

    // create list of all operations in creationGroup and dimensionsGroup
    getSetOfGroupElements();
    loadLuaFolder("actions", "operations", luaPath);

    // fetch a list of all keys from the state table
    kaguya::LuaTable globalTable(_L.globalTable());
    std::vector<kaguya::LuaRef> globalKeys = globalTable.keys();

    // list of properties to look out for in the operation
    std::set<std::string> initProperties = {
        "init",
        "command_line",
        "menu_actions",
        "icon",
        "description",
        "operation_options",
        "context_transitions"
    };

    /* Loop through all keys to search for the ones containing "Operation" in their name,
     * Call initialize operation with list of all found properties for the operation
     */
    for (kaguya::LuaRef v : globalKeys)
    {
        if (v.isType<std::string>())
        {
            std::string vkey = std::string(v.get<std::string>());

            // if Operation is found in key e.g. LineOperations
            if (vkey.find("Operation") < vkey.length() && _L[vkey].type() == _L[vkey].TYPE_TABLE) {
                kaguya::LuaTable opTable = _L[vkey];
                foundProperties[vkey] = std::set<std::string>();

                for (kaguya::LuaRef& op : opTable.keys())
                {
                    if (op.isType<std::string>())
                    {
                        std::string opkey = std::string(op.get<std::string>());

                        // if property is in properties we are looking for, add it in foundProperties
                        if (initProperties.find(opkey) != initProperties.end()) {
                            foundProperties[vkey].insert(opkey);
                        }
                    }
                }

                // determine group name for operation
                if (groupElements.at("creationGroupElements").find(vkey) != groupElements.at("creationGroupElements").end()) {
                    groupNames[vkey] = "Creation";
                }
                else if (groupElements.at("dimensionsGroupElements").find(vkey) != groupElements.at("dimensionsGroupElements").end()) {
                    groupNames[vkey] = "Dimensions";
                }
                else {
                    // if not in creation group or dimension group, it has to be in the modify group
                    groupNames[vkey] = "Modify";
                }

                initializeOperation(vkey);
            }
        }
    }

    _L["run_op"] = nullptr;
}

void OperationLoader::loadLuaFolder(const std::string folderName, const std::string& fileToSkip, const std::string& luaPath) {
    QDir folderDir((luaPath + "/" + folderName).c_str());
    QStringList luaFiles = folderDir.entryList(QStringList() << "*.lua", QDir::Files);
    for (QString str : luaFiles)
    {
        std::string filename = str.toStdString();
        // skip fileToSkip.lua as it has been already called
        if (str.toStdString() != fileToSkip) {
            _L.dofile(luaPath + "/" + folderName + "/" + filename);
        }
    }
}

void OperationLoader::getSetOfGroupElements() {
    groupElements["creationGroupElements"] = std::set<std::string>();
    groupElements["dimensionsGroupElements"] = std::set<std::string>();

    // insert elements into their respective sets
    for (kaguya::LuaRef v : _L.globalTable().keys())
    {
        if (v.isType<std::string>()) {
            std::string vkey = std::string(v.get<std::string>());

            if (vkey.find("Operation") < vkey.length()) {
                if (vkey.find("Dim") < vkey.length()) {
                    groupElements["dimensionsGroupElements"].insert(vkey);
                }
                else {
                    groupElements["creationGroupElements"].insert(vkey);
                }
            }
        }
    }
}

void OperationLoader::initializeOperation(const std::string& vkey)
{
    for (const std::string& opkey : foundProperties[vkey])
    {
        // init function
        if (opkey == "init") {
            // eg. _L["LineOperations"]["init"]()
            _L[vkey][opkey]();
        }

        // command line
        if (opkey == "command_line") {
            addOperationCommandLine(vkey, opkey);
        }

        // menu actions
        if (opkey == "menu_actions") {
            addOperationMenuAction(vkey, opkey);
        }

        // Toolbar attributes
        if (opkey == "icon") {
            addOperationIcon(vkey, opkey);
        }

        // operation icons
        if (opkey == "operation_options") {
            addOperationToolbarOptions(vkey, opkey);
        }

        // context transitions
        if (opkey == "context_transitions") {
            addContextTransitions(vkey, opkey);
        }
    }

    addContextMenuOperations(vkey);
}


void OperationLoader::addOperationCommandLine(const std::string& vkey, const std::string& opkey) {
    widgets::CliCommand* cliCommand = static_cast<MainWindow*>(qmainWindow)->cliCommand();

    if (_L[vkey][opkey].type() == _L[vkey][opkey].TYPE_STRING) {
        _L.dostring("run_op = function() run_basic_operation(" + vkey + ") end");
        cliCommand->addCommand(_L[vkey][opkey], _L["run_op"]);
    }

    if (_L[vkey][opkey].type() == _L[vkey][opkey].TYPE_TABLE) {
        std::vector<kaguya::LuaRef> commandList = _L[vkey][opkey].keys();

        for (kaguya::LuaRef commandKey : commandList) {
            std::string key = commandKey.get<std::string>();

            // if key is digits only i.e. if no key provided, connect it to default init
            if (std::find_if(key.begin(), key.end(), [](unsigned char c) {return !std::isdigit(c); }) == key.end())
            {
                // connect to default init function
                _L.dostring("run_op = function() run_basic_operation(" + vkey + ") end");
                cliCommand->addCommand(_L[vkey][opkey][commandKey].get<std::string>().c_str(), _L["run_op"]);
            }
            else {
                // connect to provided init function
                _L.dostring("run_op = function() run_basic_operation(" + vkey + ", '_init_" + _L[vkey][opkey][commandKey].get<std::string>() + "') end");
                cliCommand->addCommand(key.c_str(), _L["run_op"]);
            }
        }
    }
}

void OperationLoader::addOperationMenuAction(const std::string& vkey, const std::string& opkey) {
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    std::map<std::string, std::string> map = _L[vkey][opkey];

    for (auto element : map)
    {
        if (element.first == "default") {
            _L.dostring("run_op = function() run_basic_operation(" + vkey + ") end");
        }
        else {
            _L.dostring("run_op = function() run_basic_operation(" + vkey + ", '_init_" + element.first + "') end");
        }
        mWindow->connectMenuItem(element.second, _L["run_op"]);
    }
}

void OperationLoader::addOperationIcon(const std::string& vkey, const std::string& opkey) {
    widgets::Toolbar* toolbar = static_cast<MainWindow*>(qmainWindow)->toolbar();
    std::string icon = _L[vkey][opkey].get<std::string>();
    std::string tooltip;

    // if description not provided, use operation name
    if (foundProperties[vkey].find("description") != foundProperties[vkey].end()) {
        tooltip = _L[vkey]["description"].get<std::string>();
    }
    else {
        tooltip = vkey.substr(0, vkey.find("Operation"));
    }

    std::string iconPath = ":/icons/" + icon;
    _L.dostring("run_op = function() run_basic_operation(" + vkey + ") end");

    toolbar->addButton(vkey.c_str(), iconPath.c_str(), groupNames[vkey].c_str(), _L["run_op"], tooltip.c_str());
}

void OperationLoader::addOperationToolbarOptions(const std::string& vkey, const std::string& opkey) {
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    std::map<std::string, kaguya::LuaRef> options = _L[vkey][opkey];

    std::vector<kaguya::LuaRef> optionsList;
    for (auto element : options) {
        // operation_options for init_method
        if (element.first.find("_init") < element.first.size()) {
            std::map<std::string, kaguya::LuaRef> optionsInit = element.second;

            std::vector<kaguya::LuaRef> optionsInitList;
            for (auto elementInit : optionsInit) {
                std::map<std::string, std::string> optionInit = elementInit.second;

                std::string action = "operation_op = function() mainWindow:toolbar():addButton('cancel', ':/icons/" + optionInit["icon"] + "', 'Current operation', function() luaInterface:operation():" + optionInit["action"] + "() end, '" + elementInit.first + "') end";
                _L.dostring(action);
                optionsInitList.push_back(_L["operation_op"]);
            }

            // LINEOPERATIONS_init_pal - example key for operation options list
            mWindow->addOperationOptions(_L[vkey]["command_line"].get<std::string>() + element.first, optionsInitList);
        }
        else
        {
            // default operation_options
            std::map<std::string, std::string> option = element.second;

            std::string action = "operation_op = function() mainWindow:toolbar():addButton('cancel', ':/icons/" + option["icon"] + "', 'Current operation', function() luaInterface:operation():" + option["action"] + "() end, '" + element.first + "') end";
            _L.dostring(action);
            optionsList.push_back(_L["operation_op"]);
        }
    }

    // provide options list to mainWindow so it can run necessary function on runOperation
    if (optionsList.size() > 0) {
        mWindow->addOperationOptions(_L[vkey]["command_line"], optionsList);
    }
    _L["operation_op"] = nullptr;
}

void OperationLoader::addContextMenuOperations(const std::string& vkey) {
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    lc::ui::ContextMenuManager* contextMenuManager = ContextMenuManager::GetContextMenuManager(mWindow->contextMenuManagerId());
    contextMenuManager->addOperation(vkey, groupNames[vkey]);
}

void OperationLoader::addContextTransitions(const std::string& vkey, const std::string& opkey) {
    MainWindow* mWindow = static_cast<MainWindow*>(qmainWindow);
    lc::ui::ContextMenuManager* contextMenuManager = ContextMenuManager::GetContextMenuManager(mWindow->contextMenuManagerId());
    std::map<std::string, kaguya::LuaRef> functionNames = _L[vkey][opkey];

    for (auto fName : functionNames) {
        std::vector<kaguya::LuaRef> transitionList = _L[vkey][opkey][fName.first];
        std::vector<std::string> transList;

        for (kaguya::LuaRef transitionFunc : transitionList) {
            transList.push_back(transitionFunc.get<std::string>());
        }

        contextMenuManager->addTransition(_L[vkey]["name"], fName.first, transList);
    }
}
