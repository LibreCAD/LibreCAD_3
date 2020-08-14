#include "copymanager.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

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
        entityValue.AddMember("name", entityName, entityDocument.GetAllocator());

        rapidjson::Value propertiesList(rapidjson::kArrayType);

        for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
            rapidjson::Value propValue = propertyValue(iter->first, iter->second, entityDocument);
            propertiesList.PushBack(propValue, entityDocument.GetAllocator());
        }

        entityValue.AddMember("properties", propertiesList, entityDocument.GetAllocator());
        entityList.PushBack(entityValue, entityDocument.GetAllocator());
    }

    entityDocument.AddMember("entities", entityList, entityDocument.GetAllocator());

    std::stringstream sstream;
    rapidjson::OStreamWrapper osw(sstream);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    entityDocument.Accept(writer);

    QMimeData* mimeData = new QMimeData();
    mimeData->setText(QString(sstream.str().c_str()));
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setMimeData(mimeData);
}

void CopyManager::pasteEvent() {
    QClipboard* clipBoard = QApplication::clipboard();
    const QMimeData* mimeData = clipBoard->mimeData();

    if (mimeData->hasText()) {
        std::string jsonData = mimeData->text().toStdString();
        rapidjson::Document entityDocument;
        std::cout << "JSON data - " << jsonData << std::endl;
        entityDocument.Parse(jsonData.c_str());
        //readEntities(entityDocument);
    }
}

void CopyManager::readEntities(rapidjson::Document& document) {
    if (document.HasMember("entities")) {
        rapidjson::Value entitiesList = document["entities"].GetArray();

        for (rapidjson::Value::ConstValueIterator iter = entitiesList.Begin(); iter != entitiesList.End(); ++iter) {
            if (iter->HasMember("name")) {
                const rapidjson::Value& entityNameVal = (*iter)["name"];
                std::cout << entityNameVal.GetString() << std::endl;
            }

            //lc::entity::PropertiesMap propertiesMap;
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

void CopyManager::createEntity(const std::string& entityName, const lc::entity::PropertiesMap& propertiesList) {

}

rapidjson::Value CopyManager::propertyValue(const std::string key, const lc::entity::EntityProperty& entityProperty, rapidjson::Document& document) {
    rapidjson::Value entityProp(rapidjson::kObjectType);
    int entityType = entityProperty.which();

    rapidjson::Value keyProp;
    keyProp.SetString(key.c_str(), key.size(), document.GetAllocator());
    entityProp.AddMember("key", keyProp, document.GetAllocator());

    rapidjson::Value typeProp;
    rapidjson::Value valueProp;
    switch (entityType) {
        case 0:
        {
            // angle
            typeProp.SetString("angle", 5, document.GetAllocator());
            valueProp.SetDouble(boost::get<lc::entity::AngleProperty>(entityProperty).Get());
            break;
        }
        case 1:
        {
            // double
            typeProp.SetString("double", 6, document.GetAllocator());
            valueProp.SetDouble(boost::get<double>(entityProperty));
            break;
        }
        case 2:
        {
            // bool
            typeProp.SetString("bool", 4, document.GetAllocator());
            valueProp.SetBool(boost::get<bool>(entityProperty));
            break;
        }
        case 3:
        {
            // coordinate
            typeProp.SetString("coordinate", 10, document.GetAllocator());
            lc::geo::Coordinate coord = boost::get<lc::geo::Coordinate>(entityProperty);
            valueProp.SetObject();
            valueProp.AddMember("x", coord.x(), document.GetAllocator());
            valueProp.AddMember("y", coord.y(), document.GetAllocator());
            break;
        }
        case 4:
        {
            // text
            typeProp.SetString("text", 4, document.GetAllocator());
            std::string textProp = boost::get<std::string>(entityProperty);
            valueProp.SetString(textProp.c_str(), textProp.size(), document.GetAllocator());
            break;
        }
        case 5:
        {
            // vector coordinates
            typeProp.SetString("vector_coordinate", 17, document.GetAllocator());
            valueProp.SetArray();
            std::vector<lc::geo::Coordinate> coords = boost::get<std::vector<lc::geo::Coordinate>>(entityProperty);
            for (const lc::geo::Coordinate& coord : coords) {
                rapidjson::Value coordValue(rapidjson::kObjectType);
                coordValue.AddMember("x", coord.x(), document.GetAllocator());
                coordValue.AddMember("y", coord.y(), document.GetAllocator());
                valueProp.PushBack(coordValue, document.GetAllocator());
            }
            break;
        }
    }

    entityProp.AddMember("type", typeProp, document.GetAllocator());
    entityProp.AddMember("value", valueProp, document.GetAllocator());

    return std::move(entityProp);
}
