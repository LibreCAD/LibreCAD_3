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

                std::string getLabel();

                void setLabel(const char* newMenuLabel);

                MenuItem* getItem(const char* menuItemLabel);

                void removeItem(const char* menuItemLabel);

                void removeItem(MenuItem* item);

                void hide();

                void show();

                void teststuff();

                int getPosition() const;

                /* 0 indexed positions */
                void setPosition(int newPosition);

                void setLuaInterface(LuaInterface* luaInterfaceIn, bool setCallbacks=true);

            private:
                LuaInterface* luaInterface;
                int menuPosition;
            };
        }
    }
}
