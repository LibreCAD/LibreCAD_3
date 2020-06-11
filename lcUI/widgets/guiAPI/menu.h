#pragma once

#include <QMenu>

#include <luainterface.h>
#include "menuitem.h"

namespace lc
{
    namespace ui
    {
        namespace api
        {
            /**
            * \brief GUI widget for menu
            */
            class Menu : public QMenu
            {
                Q_OBJECT

            public:
                /**
                * \brief Menu Constructor
                * \param string menu name
                * \param pointer to QWidget parent
                */
                Menu(const char* menuName, QWidget* parent = nullptr);

                /**
                * \brief Menu Constructor
                * \param pointer to QMenuBar
                */
                Menu(QMenuBar* menuBar);

                /**
                * \brief Menu Constructor
                * \param pointer to QMenu
                */
                Menu(QMenu* menu);

                /**
                * \brief Add menu item
                * \param pointer to menu item
                */
                void addItem(MenuItem* item);

                /**
                * \brief Add menu item
                * \param string menu item name
                * \param pointer to QWidget parent
                * \return pointer to menu item
                */
                MenuItem* addItem(const char* menuItemLabel);

                /**
                * \brief Add menu item
                * \param string menu item name
                * \param LuaRef callback
                * \return pointer to menu item
                */
                MenuItem* addItem(const char* menuItemLabel, kaguya::LuaRef callback);

                /**
                * \brief Add men
                * \param pointer to menu
                */
                void addMenu(Menu* newMenu);

                /**
                * \brief Add menu item
                * \param string menu name
                * \return pointer to new menu
                */
                Menu* addMenu(const char* menuLabel);

                /**
                * \brief Get menu label
                * \return string label
                */
                std::string label() const;

                /**
                * \brief Set menu label
                * \param string new menu label
                */
                void setLabel(const char* newMenuLabel);

                /**
                * \brief Get menu item
                * \param string menu item name
                * \return pointer to menu item
                */
                MenuItem* itemByName(const char* menuItemLabel);

                /**
                * \brief Get menu item
                * \param int menu item position
                * \return pointer to menu item
                */
                MenuItem* itemByPosition(int pos);

                /**
                * \brief Get menu
                * \param string menu name
                * \return pointer to menu
                */
                Menu* menuByName(const char* menuLabel);

                /**
                * \brief Get menu item
                * \param int menu position
                * \return pointer to menu
                */
                Menu* menuByPosition(int pos);

                /**
                * \brief Remove menu item
                * \param string menu item name
                */
                void removeItem(const char* menuItemLabel);

                /**
                * \brief Remove menu item
                * \param pointer to menu item
                */
                void removeItem(MenuItem* item);

                /**
                * \brief Remove menu
                * \param string menu item name
                */
                void removeMenu(const char* menuLabel);

                /**
                * \brief Remove menu
                * \param pointer to menu
                */
                void removeMenu(Menu* menu);

                /**
                * \brief hide menu
                */
                void hide();

                /**
                * \brief show menu
                */
                void show();

                /**
                * \brief get current menu position
                * \return int position
                */
                int position() const;

                /**
                * \brief set current menu position
                * \param int new menu position
                */
                void setPosition(int newPosition);

                /*
                *   Position variable updated by other menu's setPosition
                */
                void updatePositionVariable(int newPosition);

                /**
                * \brief remove this menu
                */
                void remove();

                /**
                * \brief Ready for menu item callback connections
                */
                void enableConnections(bool setCallbacks = true);

                bool insideMenu;

            private:
                bool checkForItemOfSameLabel(const char* label, bool isMenu);

                void updateOtherPositionsAfterRemove();

            private:
                bool _connected;
                int _position;
            };
        }
    }
}
