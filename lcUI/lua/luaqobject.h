#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <QObject>
#include <QMetaMethod>

#include <kaguya/kaguya.hpp>
#include "lua/qtbridge.h"
/**
 * \brief QObject wrapper for Lua
 * Allow connecting Qt signals with Lua functions
 */
class LuaQObject : public QObject {
	public:
		/**
		 * \brief Create QObject wrapper
		 * \param object Pointer to QObject to wrap
		 */
		LuaQObject(QObject* object);
		~LuaQObject();

        /**
         * \brief Connect Qt slot to Lua function
         * \param signalId Object signal ID
         * \param slot Lua function
         * \return true if connected, false if an error happened
         */
		bool connect(int signalId, const kaguya::LuaRef& slot);

        /**
         * \brief Add signal parameter
         * \param s Lua state
         * \param type Qt QMetaType ID
         * \param arg Pointer to arg
         * Push a signal parameter to Lua stack
         */
		void pushArg(kaguya::State& s, int type, void const* arg);

        /**
         * \brief Get object name.
         * \return QObject name
         */
		std::string objectName();
        /**
         * \brief Get object name.
         * \param object Pointer to QObject
         * \return QObject name
         */
		static std::string objectName(QObject* object);

        /**
         * \brief Find children by name
         * \param name Children name
         * \return Pointer to QObject
         */
		QObject* findChild(const std::string& name);
        /**
         * \brief Find children by name
         * \param object Pointer to QObject
         * \param name Children name
         * \return Pointer to QObject
         */
		static QObject* findChild(QObject* object, const std::string& name);

        /**
         * \brief Return object validity
         * \return true if QOaject still exists
         */
		bool valid();

        /**
         * \brief Internal Qt function
         * This function is called when a slot of this object is called.
         * It gets the arguments and call the Lua function
         */
		int qt_metacall(QMetaObject::Call c, int id, void **a) override;

	private:
		QObject* _object;

		int _slotId;
		int _signalId;
		kaguya::LuaRef _slotFunction;

		bool _valid;
};

DECLARE_SHORT_SHARED_PTR(LuaQObject)