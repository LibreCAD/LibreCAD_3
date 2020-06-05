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

                ToolbarButton* addButton(const char* buttonName, const char* buttonIcon);

                ToolbarButton* addButton(const char* buttonName, const char* buttonIcon, kaguya::LuaRef callback);

                void addWidget(QWidget* widget);

                std::string label() const;

                void setLabel(const char* newGroupName);

                ToolbarButton* getButton(const char* buttonName);

                std::vector<ToolbarButton*> getAllButtons();

                void removeButton(const char* buttonName);

                void removeButton(ToolbarButton* button);

                void setLuaInterface(LuaInterface* luaInterface);

            private:
                LuaInterface* luaInterface;
                const int width;
                int count;
            };
        }
    }
}
