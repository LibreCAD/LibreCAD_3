#pragma once

#include <QGroupBox>

#include "toolbarbutton.h"

#include "luainterface.h"

namespace lc
{
    namespace ui
    {
        namespace api
        {
            class ToolbarGroup : public QGroupBox
            {
                Q_OBJECT

            public:
                ToolbarGroup(const char* groupName, int width = 3, QWidget* parent = nullptr);

                void addButton(ToolbarButton* button);

                void setLuaInterface(LuaInterface* luaInterface);

            private:
                LuaInterface* luaInterface;
                int width;
            };
        }
    }
}
