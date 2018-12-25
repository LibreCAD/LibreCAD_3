#pragma once

#include <lclua.h>
#include <kaguya/kaguya.hpp>

namespace lc {
    namespace lua {
        class LuaCustomEntityManager {
            public:
                static LuaCustomEntityManager& getInstance() {
                    static LuaCustomEntityManager _instance;

                    return _instance;
                }

                LuaCustomEntityManager(LuaCustomEntityManager const&) = delete;

                void operator=(LuaCustomEntityManager const&) = delete;

                virtual ~LuaCustomEntityManager();

                /**
                 * @brief Register a new plugin which handle custom entities
                 * @param name Name of the plugin
                 * @param onNewWaitingEntityFunction Function called when there are entities which needs to be recreated by the plugin
                 */
                void registerPlugin(const std::string& name, kaguya::LuaRef onNewWaitingEntityFunction);

                /**
                 * @brief Remove all registered plugins
                 * This should be called before the Lua instance get deleted
                 */
                void removePlugins();

            private:
                LuaCustomEntityManager();

                void onNewWaitingEntity(const lc::event::NewWaitingCustomEntityEvent& event);

                std::map<std::string, kaguya::LuaRef> _plugins;
        };
    }
}