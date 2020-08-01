#pragma once

#include <QMenu>
#include <cad/base/cadentity.h>
#include <kaguya/kaguya.hpp>

#include "widgets/guiAPI/menu.h"

namespace lc
{
    namespace ui
    {
        /**
         * \brief Context Menu Manager
         */
        class ContextMenuManager
        {
        public:
            /**
             * \brief Constructor for ContextMenuManager
             * \param luaState lua state
             */
            ContextMenuManager(lua_State* luastate);

            /**
             * \brief Generate menu for the selected entities / no entity selected
             * \param vector of selected entities cad entity shared pointers
             */
            void generateMenu(api::Menu* menu, std::vector<lc::entity::CADEntity_CSPtr> selectedEntities);

            /**
             * \brief Add operation to the list (callled by operation loader)
             * \param string Operation Name (key)
             * \param string group name the operation belongs to
             */
            void addOperation(const std::string& key, const std::string& groupName);

        private:
            kaguya::State _L;
            std::map<std::string, std::vector<std::string>> _operationMap;
        };
    }
}
