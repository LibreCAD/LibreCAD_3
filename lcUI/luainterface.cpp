#include <lclua.h>
#include <managers/luacustomentitymanager.h>
#include "luainterface.h"

#include "mainwindow.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"

using namespace lc::ui;

LuaInterface::LuaInterface() :
	_pluginManager(_L.state(), "gui") {
}

LuaInterface::~LuaInterface() {
	_luaQObjects.clear();
    _events.clear();

    lc::lua::LuaCustomEntityManager::getInstance().removePlugins();
}

void LuaInterface::initLua(QMainWindow* mainWindow) {
	auto lcLua = lc::lua::LCLua(_L.state());
    lcLua.setF_openFileDialog(&LuaInterface::openFileDialog);
    lcLua.addLuaLibs();
	lcLua.importLCKernel();

    luaOpenQtBridge(_L.state());

    _L["luaInterface"] = this;
    _L["mainWindow"] = static_cast<lc::ui::MainWindow*>(mainWindow);

    QString luaFile = QCoreApplication::applicationDirPath() + "/path.lua";
    bool s = _L.dofile(luaFile.toStdString().c_str());

    std::string lua_path = _L["lua_path"];
    loadLuaOperations(lua_path, mainWindow);

    if (s) {
		const char* out = lua_tostring(_L.state(), -1);
		if (out == nullptr) {
			LOG_WARNING << "Lua output null" << std::endl;
		}
		else {
			LOG_INFO << "Lua output:" << out << std::endl;
		}
        lua_pop(_L.state(), 1);
    }

	_pluginManager.loadPlugins();
}

bool LuaInterface::luaConnect(
	QObject* sender,
	const std::string& signalName,
	const kaguya::LuaRef& slot)
{
	int signalId = sender->metaObject()->indexOfSignal(signalName.c_str());
	
	if(signalId < 0) {
		std::cout << "No such signal " << signalName << std::endl;
	}
	else {
		auto lqo = std::make_shared<LuaQObject>(sender);
		_luaQObjects.push_back(lqo);

		auto connected = lqo->connect(signalId, slot);

		cleanInvalidQObject();

		return connected;
	}

	return false;
}

QWidget* LuaInterface::loadUiFile(const char* fileName) {
	QUiLoader uiLoader;
	QFile file(fileName);
    file.open(QFile::ReadOnly);

    QWidget* widget = uiLoader.load(&file);

    file.close();

    return widget;
}

void LuaInterface::cleanInvalidQObject() {
	_luaQObjects.erase(std::remove_if(_luaQObjects.begin(),
									  _luaQObjects.end(),
							  [](LuaQObject_SPtr lqo){
								  return !lqo->valid();
							  }),
					   _luaQObjects.end());
}

bool LuaInterface::qtConnect(QObject *sender, const std::string& signalName, QObject *receiver, const std::string& slotName) {
	int signalId = sender->metaObject()->indexOfSignal(signalName.c_str());
	if(signalId < 0) {
		std::cout << "No such signal " << signalName << std::endl;
	}

	int slotId = receiver->metaObject()->indexOfSlot(slotName.c_str());
	if(slotId < 0) {
		std::cout << "No such slot " << slotName << std::endl;
	}

	return QMetaObject::connect(sender, signalId, receiver, slotId) != nullptr;
}

void LuaInterface::hideUI(bool hidden) {
	_L["hideUI"] = hidden;
}

lua_State* LuaInterface::luaState() {
	return _L.state();
}

std::vector<std::string> LuaInterface::pluginList(const char* path) {
    std::vector<std::string> plugins;
    QDir dir(path);

    auto list = dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    for(const auto& dir : list) {
        plugins.push_back(dir.toStdString());
    }

    return plugins;
}

FILE* LuaInterface::openFileDialog(bool isOpening, const char* description, const char* mode) {
    QString path;

    if(isOpening) {
        path = QFileDialog::getOpenFileName(nullptr, (std::string("Open ") + description).c_str());
    }
    else {
        path = QFileDialog::getSaveFileName(nullptr, (std::string("Save ") + description).c_str());
    }

    if(path.isEmpty()) {
        return nullptr;
    }

    return fopen(path.toStdString().c_str(), mode);
}

kaguya::LuaRef LuaInterface::operation() {
    return _operation;
}

void LuaInterface::setOperation(kaguya::LuaRef operation) {
    _operation = std::move(operation);
}

void LuaInterface::registerEvent(const std::string& event, const kaguya::LuaRef& callback) {
    if(callback.type() == LUA_TTABLE && callback["onEvent"].isNilref()) {
        return;
    }

    _events[event].push_back(callback);
}

void LuaInterface::deleteEvent(const std::string& event, const kaguya::LuaRef& callback) {
    auto it = std::find(_events[event].begin(), _events[event].end(), callback);

    if(it != _events[event].end()) {
        _events[event].erase(it);
    }
}

void LuaInterface::triggerEvent(const std::string& event, kaguya::LuaRef args) {
    auto events = _events[event];
    for(auto eventCallback : events) {
        if(eventCallback.type() == LUA_TFUNCTION) {
            eventCallback(event, args);
        }
        else if(eventCallback.type() == LUA_TTABLE) {
            eventCallback["onEvent"](eventCallback, event, args);
        }
    }
}

void LuaInterface::loadLuaOperations(const std::string& lua_path, QMainWindow* mainWindow) {
    // for global functions like run_operation, message etc
    _L.dofile(lua_path + "/ui/operations.lua");

    // call operations and createoperations first
    _L.dofile(lua_path + "/actions/operations.lua");
    _L.dofile(lua_path + "/createActions/createOperations.lua");

    // run all lua files in createActions folder
    QDir createActionsDir((lua_path + "/createActions").c_str());
    QStringList luaFiles = createActionsDir.entryList(QStringList() << "*.lua", QDir::Files);
    for (QString str : luaFiles)
    {
        std::string filename = str.toStdString();
        // skip createOperations.lua as it has been already called
        if (filename != "createOperations") {
            _L.dofile(lua_path + "/createActions/" + filename);
        }
    }

    // create list of all operations in creationGroup and dimensionsGroup
    std::map<std::string, std::set<std::string>> groupElements;
    groupElements["creationGroupElements"] = std::set<std::string>();
    groupElements["dimensionsGroupElements"] = std::set<std::string>();

    // insert elements into their respective sets
    for (kaguya::LuaRef v : _L.globalTable().keys())
    {
        if (v.isType<std::string>()){
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

    // run all lua files in actions folder
    QDir actionsDir((lua_path + "/actions").c_str());
    QStringList luaFiles1 = actionsDir.entryList(QStringList() << "*.lua", QDir::Files);
    for (QString str : luaFiles1)
    {
        // skip operations.lua as it has been already called
        if (str.toStdString() != "operations") {
            _L.dofile(lua_path + "/actions/" + str.toStdString());
        }
    }

    // fetch a list of all keys from the state table
    kaguya::LuaTable globalTable(_L.globalTable());
    std::vector<kaguya::LuaRef> globalKeys = globalTable.keys();

    // count of widgets for proper positioning in toolbar group
    std::map<std::string, int> widgetCount;
    widgetCount["creationWidgetCount"] = 0;
    widgetCount["dimWidgetCount"] = 0;
    widgetCount["modifyWidgetCount"] = 0;

    // list of properties to look out for in the operation
    std::set<std::string> initProperties = {
        "init",
        "command_line",
        "menu_actions",
        "icon",
        "description"
    };

    /* Loop through all keys to search for the ones containing "Operation" in their name,
     * Call initialize operation with list of all found properties for the operation
     */
    for (kaguya::LuaRef v : globalKeys)
    {
        if (v.isType<std::string>())
        {
            std::string vkey = std::string(v.get<std::string>());

            if (vkey.find("Operation") < vkey.length()) {
                kaguya::LuaTable opTable = _L[vkey];

                std::set<std::string> foundProperties;

                for(kaguya::LuaRef& op : opTable.keys())
                {
                    if (op.isType<std::string>())
                    {
                        std::string opkey = std::string(op.get<std::string>());

                        if (initProperties.find(opkey) != initProperties.end()) {
                            foundProperties.insert(opkey);
                        }
                    }
                }

                initializeOperation(vkey, foundProperties, widgetCount, groupElements, mainWindow);
            }
        }
    }

    _L["run_op"] = nullptr; 
}

void LuaInterface::initializeOperation(const std::string& vkey, const std::set<std::string>& foundProperties,
    std::map<std::string, int>& widgetCount, const std::map<std::string, std::set<std::string>>& groupElements, QMainWindow* mainWindow)
{
    // mainWindow to connect to command line and menu
    lc::ui::MainWindow* mWindow = static_cast<lc::ui::MainWindow*>(mainWindow);
    lc::ui::widgets::CliCommand* cliCommand = mWindow->getCliCommand();
    lc::ui::widgets::Toolbar* toolbar = mWindow->getToolbar();

    for (const std::string& opkey : foundProperties)
    {
        // init function
        if (opkey == "init") {
            // eg. _L["LineOperations"]["init"]()
            _L[vkey][opkey]();
        }

        // command line
        if (opkey == "command_line") {
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
                        cliCommand->addCommand(_L[vkey][opkey][commandKey].get<std::string>(), _L["run_op"]);
                    }
                    else {
                        // connect to provided init function
                        _L.dostring("run_op = function() run_basic_operation(" + vkey + ", '_init_" + key + "') end");
                        cliCommand->addCommand(_L[vkey][opkey][commandKey].get<std::string>(), _L["run_op"]);
                    }
                }
            }
        }

        //menu actions
        if (opkey == "menu_actions")
        {
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

        // Toolbar attributes
        if (opkey == "icon") {
            std::string icon = _L[vkey][opkey].get<std::string>();

            std::string tooltip;
            
            // if description not provided, use operation name
            if (foundProperties.find("description") != foundProperties.end()) {
                tooltip = _L[vkey]["description"].get<std::string>();
            }
            else {
                tooltip = vkey.substr(0, vkey.find("Operation"));
            }
            
            std::string iconPath = ":/icons/" + icon;
            _L.dostring("run_op = function() run_basic_operation(" + vkey + ") end");
            std::string groupName;
            int row;
            int col;

            const int widgetsPerRow = 3;

            if (groupElements.at("creationGroupElements").find(vkey) != groupElements.at("creationGroupElements").end()) {
                groupName = "Creation";
                col = widgetCount["creationWidgetCount"] / widgetsPerRow;
                row = widgetCount["creationWidgetCount"] % widgetsPerRow;
                widgetCount["creationWidgetCount"]++;
            }
            else if (groupElements.at("dimensionsGroupElements").find(vkey) != groupElements.at("dimensionsGroupElements").end()) {
                groupName = "Dimensions";
                col = widgetCount["dimWidgetCount"] / widgetsPerRow;
                row = widgetCount["dimWidgetCount"] % widgetsPerRow;
                widgetCount["dimWidgetCount"]++;
            }
            else {
                // if not in creation group or dimension group, it has to be in the modify group
                groupName = "Modify";
                col = widgetCount["modifyWidgetCount"] / widgetsPerRow;
                row = widgetCount["modifyWidgetCount"] % widgetsPerRow;
                widgetCount["modifyWidgetCount"]++;
            }

            toolbar->addButton("", iconPath.c_str(), groupName.c_str(), row, col, _L["run_op"], tooltip.c_str());
        }
    }
}
