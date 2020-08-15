#pragma once

#include <cad/base/cadentity.h>
#include <rapidjson/document.h>
#include <cadmdichild.h>

namespace lc {
    namespace ui {
        class CopyManager
        {
        public:
            CopyManager(CadMdiChild* cadMdiChild = nullptr);

            void copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities);

            void pasteEvent();

        private:
            void readEntities(rapidjson::Document& document);

            void createEntity(const std::string& entityName, const lc::entity::PropertiesMap& propertiesList);

            rapidjson::Value propertyValue(const std::string key, const lc::entity::EntityProperty& entityProperty, rapidjson::Document& document);

            lc::entity::EntityProperty getEntityPropertyFromJSONValue(const rapidjson::Value& value);

        private:
            CadMdiChild* _cadMdiChild;
        };
    }
}
