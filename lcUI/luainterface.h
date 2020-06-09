#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <set>
#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QtUiTools/QUiLoader>
#include <QCoreApplication>
#include <managers/pluginmanager.h>

#include <kaguya/kaguya.hpp>
#include "lua/qtbridge.h"

#include "lua/luaqobject.h"

namespace lc {
	namespace ui {
		/**
		 * \brief Lua initializer
		 */
		class LuaInterface {
			public:
				/**
                 * \brief Create Lua instance
                 */
				LuaInterface();

				~LuaInterface();

				/**
                 * \brief Read and execute Lua files
                 */
				void initLua(QMainWindow* mainWindow);

                /**
                 * \brief load operations from lua scripts and run init functions
                 * \param lua_path string path to lcUI folder
                 */
                void loadLuaOperations(const std::string& lua_path, QMainWindow* mainWindow);

				/**
                 * \brief Connect Qt signal with Lua function
                 * \param sender Pointer to sender object
                 * \param signalName Name of the signal with parameters
                 * \param slot Lua function
                 * \return true if connected, false if an error happened
                 */
				bool luaConnect(
						QObject* sender,
						const std::string& signalName,
						const kaguya::LuaRef& slot
				);

                bool luaDisconnect(
                    QObject* sender,
                    const std::string& signalName,
                    const kaguya::LuaRef& slot
                );

				/**
                 * \brief Connect Qt signal with Qt slot
                 * \param sender Pointer to sender object
                 * \param signalName Name of the signal with parameters
                 * \param receiver Pointer to receiver object
                 * \param slotName Name of the slot with parameters
                 */
				bool qtConnect(QObject* sender,
							   const std::string& signalName,
							   QObject* receiver,
							   const std::string& slotName);

				/**
                 * \brief Remove all connections that aren't valid anymore.
                 */
				void cleanInvalidQObject();

				/**
                 * \brief Load Qt widget from .ui file
                 * \param fileName full path to .ui file
                 */
				static QWidget* loadUiFile(const char* fileName);

				/**
                 * \brief Return a list of plugins
                 * \param path Path of plugins
                 * \return List of strings containing the name of plugins
                 */
				std::vector<std::string> pluginList(const char* path);

				/**
                 * \brief Hide the window.
                 * It needs to be used before initLua(), this is used in unit tests.
                 */
				void hideUI(bool hidden);

				/**
                 * \brief Returns current Lua state.
                 * This is used for unit tests.
                 */
				lua_State* luaState();

				static FILE* openFileDialog(bool isOpening, const char* description, const char* mode);

				kaguya::LuaRef operation();

				void setOperation(kaguya::LuaRef);

                void finishOperation();

				void registerEvent(const std::string& event, const kaguya::LuaRef& callback);

				void deleteEvent(const std::string& event, const kaguya::LuaRef& callback);

				void triggerEvent(const std::string& event, kaguya::LuaRef args);

            private:
                /**
                 * \brief Helper function to initialize each operation on the basis of found properties
                 * \param vkey - Operation key for state table
                 *        foundProperties - List of found properties of operation in lua file
                 *        widgetCount - Count of widgets in toolbar groups
                 *        groupElements - List of widgets in toolbar groups
                 *        mainWindow - to cast to MainWindow and access clicommand,toolbar and cadmdichild
                 */
                void initializeOperation(const std::string& vkey, const std::set<std::string>& foundProperties, const std::string& groupName,
                    QMainWindow* mainWindow);

                /**
                 * \brief make common functions available globally and register finish events
                 */
                void registerGlobalFunctions(QMainWindow* mainWindow);

                void loadLuaFolder(const std::string folderName, const std::string& fileToSkip, const std::string& luaPath);

                std::map<std::string, std::set<std::string>> getSetOfGroupElements();

                void addOperationCommandLine(const std::string& vkey, const std::string& opkey, QMainWindow* mainWindow);

                void addOperationMenuAction(const std::string& vkey, const std::string& opkey, QMainWindow* mainWindow);

                void addOperationIcon(const std::string& vkey, const std::string& opkey, QMainWindow* mainWindow, const std::set<std::string>& foundProperties,
                    const std::string& groupName);

                void addOperationToolbarOptions(const std::string& vkey, const std::string& opkey, QMainWindow* mainWindow);

			private:
				kaguya::State _L;
				QList<QSharedPointer<LuaQObject>> _luaQObjects;
				lc::lua::PluginManager _pluginManager;
				kaguya::LuaRef _operation;
				std::map<std::string, std::vector<kaguya::LuaRef>> _events;
		};
	}
}
