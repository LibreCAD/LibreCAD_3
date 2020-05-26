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

                MenuItem* addItem(const char* menuItemName);

                std::string getLabel();

                MenuItem* getMenuItem(const char* menuItemLabel);

                void setLuaInterface(LuaInterface* luaInterfaceIn);

            private:
                LuaInterface* luaInterface;
            };
        }
    }
}
