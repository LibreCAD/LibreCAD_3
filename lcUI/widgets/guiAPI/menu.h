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
            class Menu : public QMenu
            {
                Q_OBJECT

            public:
                Menu(const char* menuName, QWidget* parent = nullptr);

                Menu(QMenuBar* menuBar);

                Menu(QMenu* menu);

                void addItem(MenuItem* item);

                MenuItem* addItem(const char* menuItemLabel);

                MenuItem* addItem(const char* menuItemLabel, kaguya::LuaRef callback);

                void addMenu(Menu* newMenu);

                Menu* addMenu(const char* menuLabel);

                std::string label();

                void setLabel(const char* newMenuLabel);

                MenuItem* getItem(const char* menuItemLabel);

                MenuItem* getItem(int pos);

                Menu* getMenu(const char* menuLabel);

                Menu* getMenu(int pos);

                void removeItem(const char* menuItemLabel);

                void removeItem(MenuItem* item);

                void removeMenu(const char* menuLabel);

                void removeMenu(Menu* menu);

                void hide();

                void show();

                int position() const;

                void setPosition(int newPosition);

                /*
                *   Position variable updated by other menu's setPosition
                */
                void updatePositionVariable(int newPosition);

                void remove();

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
