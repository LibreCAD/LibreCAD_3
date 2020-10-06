#pragma once

#include <cad/base/cadentity.h>
#include <rapidjson/document.h>
#include <cadmdichild.h>

namespace lc {
    namespace ui {
        /**
        * \brief Copy Manager
        */
        class CopyManager
        {
        public:
            /**
            * \brief CopyManager constructor
            * \param pointer to CadMdiChild
            */
            CopyManager(CadMdiChild* cadMdiChild = nullptr);

            /**
            * \brief Copy event, copy entities to the clipboard
            * \param pointer to vector of selected cad entities
            */
            void copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities);

            /**
            * \brief Paste the entities to the window
            */
            void pasteEvent();

        private:
            /**
            * \brief Read entities from the document
            */
            void readEntities(rapidjson::Document& document);

            /**
            * \brief Create and add entity from the given properties list
            */
            void createEntity(const std::string& entityName, const lc::entity::PropertiesMap& propertiesList);

            /**
            * \brief Create entities with extra properties (lwpolyline)
            */
            void createOtherEntity(const std::string& entityName, const lc::entity::PropertiesMap& propertiesList, const rapidjson::Value& otherProperties);

            /**
            * \brief Serialize entity property to JSON
            */
            rapidjson::Value propertyValue(const std::string key, const lc::entity::EntityProperty& entityProperty, rapidjson::Document& document);

            /**
            * \brief Deserialize entity property from json
            */
            lc::entity::EntityProperty getEntityPropertyFromJSONValue(const rapidjson::Value& value);

            /**
            * \brief Deserialize entities with extra properties
            */
            rapidjson::Value getCustomProperties(const std::string& entityName, lc::entity::CADEntity_CSPtr entity, rapidjson::Document& document);

        private:
            CadMdiChild* _cadMdiChild;
        };
    }
}
