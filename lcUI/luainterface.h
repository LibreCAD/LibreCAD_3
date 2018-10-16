#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <iostream>

#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QtUiTools/QUiLoader>
#include <QCoreApplication>
#include <managers/pluginmanager.h>
#include "lua/luaqobject.h"

#include "lua-intf/LuaIntf/LuaIntf.h"
#include "lua/qtbridge.h"

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
				void initLua();

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
						const LuaIntf::LuaRef& slot
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
				static std::shared_ptr<QWidget> loadUiFile(const char* fileName);

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
				LuaIntf::LuaState luaState();

				static FILE* openFileDialog(bool isOpening, const char* description, const char* mode);

				LuaIntf::LuaRef operation(unsigned int windowID);

				void setOperation(unsigned int windowID, LuaIntf::LuaRef);

				void registerEvent(const std::string& event, const LuaIntf::LuaRef& callback);

				void deleteEvent(const std::string& event, const LuaIntf::LuaRef& callback);

				void triggerEvent(const std::string& event, LuaIntf::LuaRef args);

			private:
				LuaIntf::LuaState _L;
				std::vector<LuaQObject_SPtr> _luaQObjects;
				lc::lua::PluginManager _pluginManager;
				std::map<unsigned int, LuaIntf::LuaRef> _operations;
				std::map<std::string, std::vector<LuaIntf::LuaRef>> _events;
		};
	}
}