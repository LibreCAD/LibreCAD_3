#pragma once

#include <luainterface.h>

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
                * \param LuaRef callback
                * \param pointer to QWidget parent
                */
                MenuItem(const char* menuItemName, kaguya::LuaRef callback, QWidget* parent = nullptr);

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
                * \param LuaRef callback
                */
                void addCallback(kaguya::LuaRef callback);

                /**
                * \brief add named callback for item
                * \param string callback name to identiy callback
                * \param LuaRef callback
                */
                void addCallback(const char* cb_name, kaguya::LuaRef callback);

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
                * \brief Ready for menu item callback connections
                */
                void enableConnections(bool setCallbacks = true);

            signals:
                void connectToCallback(lc::ui::api::MenuItem*, const std::string&, kaguya::LuaRef&);

                void disconnectCallback(lc::ui::api::MenuItem*, const std::string&, kaguya::LuaRef&);

            private:
                void updateOtherPositionsAfterRemove();

            private:
                bool _connected;
                std::vector<kaguya::LuaRef> callbacks;
                std::unordered_map<std::string, int> namedCallbacks;
                int _position;
            };
        }
    }
}
