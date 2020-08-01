#pragma once

#include <QMenu>
#include <cad/base/cadentity.h>
#include <kaguya/kaguya.hpp>

#include "widgets/guiAPI/menu.h"

namespace lc
{
    namespace ui
    {
        class ContextMenuManager
        {
        public:
            ContextMenuManager(lua_State* luastate);

            void generateMenu(api::Menu* menu, std::vector<lc::entity::CADEntity_CSPtr> selectedEntities);

        private:
            kaguya::State _L;
        };
    }
}
