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
            class MenuItem : public QAction
            {
                Q_OBJECT

            public:
                MenuItem(const char* menuItemName, kaguya::LuaRef callback, QWidget* parent = nullptr);

                MenuItem(const char* menuItemName, QWidget* parent = nullptr);

                std::string label();

                void setLabel(const char* newLabel);

                void addCallback(kaguya::LuaRef callback);

                void addCallback(const char* cb_name, kaguya::LuaRef callback);

                void removeCallback(const char* cb_name);
                
                void hide();

                void show();

                int position() const;

                void setPosition(int newPosition);

                /*
                *   Position variable updated by other menu item's setPosition
                */
                void updatePositionVariable(int pos);

                void remove();

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
