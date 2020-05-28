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

                std::string getLabel();

                void setLabel(const char* newMenuLabel);

                MenuItem* getItem(const char* menuItemLabel);

                MenuItem* getItem(int pos);

                void removeItem(const char* menuItemLabel);

                void removeItem(MenuItem* item);

                void hide();

                void show();

                int getPosition() const;

                void setPosition(int newPosition);

                /*
                *   Position variable updated by other menu's setPosition
                */
                void updatePositionVariable(int newPosition);

                void remove();

                void setLuaInterface(LuaInterface* luaInterfaceIn, bool setCallbacks=true);

            private:
                LuaInterface* luaInterface;
                int position;
            };
        }
    }
}
