#pragma once

#include <cad/base/cadentity.h>
#include <rapidjson/document.h>

namespace lc {
    namespace ui {
        class CopyManager
        {
        public:
            CopyManager();

            void copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities);

            void pasteEvent();

        private:
            void readEntities(rapidjson::Document& document);
        };
    }
}
