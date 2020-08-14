#include "copymanager.h"

#include <QApplication>
#include <QClipboard>

#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <cad/base/cadentity.h>
#include "widgets/guiAPI/entitypickervisitor.h"

#include <iostream>
#include <sstream>

using namespace lc::ui;

CopyManager::CopyManager() {

}

void CopyManager::copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities) {
    //QClipboard* clipBoard = QApplication::clipboard();
    //clipBoard->setText(QString("Selected Entities - ") + QString::number(cadEntities.size()));

    lc::ui::api::EntityPickerVisitor entityPickerVisitor;

    rapidjson::Document entityDocument;
    entityDocument.Parse("{}");
    rapidjson::Value entityList(rapidjson::kArrayType);

    for (lc::entity::CADEntity_CSPtr cadEntity : cadEntities) {
        lc::entity::PropertiesMap propertiesMap = cadEntity->availableProperties();
        rapidjson::Value entityValue(rapidjson::kObjectType);

        cadEntity->dispatch(entityPickerVisitor);

        rapidjson::Value entityName;
        std::string entityNameStr = entityPickerVisitor.getEntityInformation();
        entityName.SetString(entityNameStr.c_str(), entityNameStr.size(), entityDocument.GetAllocator());
        entityList.AddMember("name", entityName, entityDocument.GetAllocator());

        rapidjson::Value propertiesList(rapidjson::kArrayType);

        for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
            rapidjson::Value propertyName;
            propertyName.SetString(iter->first.c_str(), iter->first.size(), entityDocument.GetAllocator());
            propertiesList.PushBack(propertyName, entityDocument.GetAllocator());
        }

        entityValue.AddMember("properties", propertiesList, entityDocument.GetAllocator());
        entityList.PushBack(entityValue, entityDocument.GetAllocator());
    }

    entityDocument.AddMember("entities", entityList, entityDocument.GetAllocator());

    std::stringstream sstream;
    rapidjson::OStreamWrapper osw(sstream);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    entityDocument.Accept(writer);

    std::string contents = sstream.str();
    std::cout << contents << std::endl;
}

void CopyManager::pasteEvent() {
    //QClipboard* clipBoard = QApplication::clipboard();
    //std::cout << "Copied text - " << clipBoard->text().toStdString() << std::endl;
}

void CopyManager::readEntities(rapidjson::Document& document) {
    if (document.HasMember("entities")) {
        rapidjson::Value entitiesList = document["entities"].GetArray();

        for (rapidjson::Value::ConstValueIterator iter = entitiesList.Begin(); iter != entitiesList.End(); ++iter) {
            if (iter->HasMember("name")) {
                const rapidjson::Value& entityNameVal = (*iter)["name"];
                std::cout << entityNameVal.GetString() << std::endl;
            }

            if (iter->HasMember("properties")) {
                const rapidjson::Value& propertiesList = (*iter)["properties"];

                for (rapidjson::Value::ConstValueIterator propertyIter = propertiesList.Begin(); propertyIter != propertiesList.End(); ++propertyIter) {
                    std::string propertyName = propertyIter->GetString();
                    std::cout << "Reading property - " << propertyName << std::endl;
                }
            }
        }
    }
}
