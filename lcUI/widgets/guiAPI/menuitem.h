#pragma once

// Phase 4 PR-4 — MenuItem stores neutral ScriptCallbacks instead of
// raw kaguya::LuaRef.  Lua callers still see LuaRef; guibridge.cpp
// wraps via lc::lua::makeLuaCallback().  kaguya include removed here.
#include <lcscripting/scriptcallback.h>

#include <unordered_map>
#include <QAction>

namespace lc
{
namespace ui
{
namespace api
{
/**
* \brief GUI widget for menu item
*/
class MenuItem : public QAction
{
    Q_OBJECT

public:
    /**
    * \brief Menu item Constructor
    * \param string menu item name
    * \param ScriptCallback callback (Lua adapter wraps at the guibridge)
    * \param pointer to QWidget parent
    */
    MenuItem(const char* menuItemName, lc::scripting::ScriptCallback callback,
             QWidget* parent = nullptr);

    /**
    * \brief Menu item Constructor
    * \param string menu item name
    * \param pointer to QWidget parent
    */
    MenuItem(const char* menuItemName, QWidget* parent = nullptr);

    /**
    * \brief get item label
    * \return string label
    */
    std::string label();

    /**
    * \brief set item label
    * \param string new label
    */
    void setLabel(const char* newLabel);

    /**
    * \brief add callback for item
    */
    void addCallback(lc::scripting::ScriptCallback callback);

    /**
    * \brief add checked callback for item
    */
    void addCheckedCallback(lc::scripting::ScriptCallback callback);

    /**
    * \brief add named callback for item
    * \param string callback name to identify callback
    */
    void addCallback(const char* cb_name, lc::scripting::ScriptCallback callback);

    /**
    * \brief remove named callback
    * \param string name of callback to be removed
    */
    void removeCallback(const char* cb_name);

    /**
    * \brief hide item
    */
    void hide();

    /**
    * \brief show item
    */
    void show();

    /**
    * \brief get current menu item position
    * \return int position
    */
    int position() const;

    /**
    * \brief set current menu item position
    * \param int new menu position
    */
    void setPosition(int newPosition);

    /*
    *   Position variable updated by other menu item's setPosition
    */
    void updatePositionVariable(int pos);

    /**
    * \brief remove this item
    */
    void remove();

    /**
    * \brief Set menu item to be checkable
    */
    void setCheckable(bool checkable);

    /**
    * \brief Set menu item to be checked
    */
    void setChecked(bool checked);

public slots:
    void itemTriggered();

    void itemToggled(bool toggle);

private:
    void updateOtherPositionsAfterRemove();

    void updateItemPositionsAfterSet(QList<QAction*>& items);

private:
    std::vector<lc::scripting::ScriptCallback> callbacks;
    std::vector<lc::scripting::ScriptCallback> _checkedCallbacks;
    std::unordered_map<std::string, int> namedCallbacks;
    int _position;
};
}
}
}
