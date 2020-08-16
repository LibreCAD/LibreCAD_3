#include "copymanager.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <cad/base/cadentity.h>
#include "widgets/guiAPI/entitynamevisitor.h"

#include <iostream>
#include <sstream>

using namespace lc::ui;

CopyManager::CopyManager(CadMdiChild* cadMdiChild) 
    :
    _cadMdiChild(cadMdiChild)
{}

void CopyManager::copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities) {
    lc::ui::api::EntityNameVisitor entityNameVisitor;

    rapidjson::Document entityDocument;
    entityDocument.Parse("{}");
    rapidjson::Value entityList(rapidjson::kArrayType);

    for (lc::entity::CADEntity_CSPtr cadEntity : cadEntities) {
        lc::entity::PropertiesMap propertiesMap = cadEntity->availableProperties();
        rapidjson::Value entityValue(rapidjson::kObjectType);

        cadEntity->dispatch(entityNameVisitor);

        rapidjson::Value entityName;
        std::string entityNameStr = entityNameVisitor.getEntityInformation();
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
        entityDocument.Parse(jsonData.c_str());
        readEntities(entityDocument);
    }
}

void CopyManager::readEntities(rapidjson::Document& document) {
    if (document.HasMember("entities")) {
        rapidjson::Value entitiesList = document["entities"].GetArray();

        for (rapidjson::Value::ConstValueIterator iter = entitiesList.Begin(); iter != entitiesList.End(); ++iter) {
            std::string entityName;
            lc::entity::PropertiesMap propertiesMap;
            
            if (iter->HasMember("name")) {
                const rapidjson::Value& entityNameVal = (*iter)["name"];
                entityName = entityNameVal.GetString();
            }

            if (iter->HasMember("properties")) {
                const rapidjson::Value& propertiesList = (*iter)["properties"];

                for (rapidjson::Value::ConstValueIterator propertyIter = propertiesList.Begin(); propertyIter != propertiesList.End(); ++propertyIter) {
                    std::string propertyKey;
                    propertyKey = (*propertyIter)["key"].GetString();
                    propertiesMap[propertyKey] = getEntityPropertyFromJSONValue(*propertyIter);
                }
            }

            createEntity(entityName, propertiesMap);
        }
    }
}

lc::entity::EntityProperty CopyManager::getEntityPropertyFromJSONValue(const rapidjson::Value& value) {
    std::string propertyType = value["type"].GetString();

    if (propertyType == "angle") {
        return lc::entity::AngleProperty(value["value"].GetDouble());
    }
    else if (propertyType == "double") {
        return value["value"].GetDouble();
    }
    else if (propertyType == "bool") {
        return value["value"].GetBool();
    }
    else if (propertyType == "coordinate") {
        const rapidjson::Value& val = value["value"];
        double x = val["x"].GetDouble();
        double y = val["y"].GetDouble();
        return lc::geo::Coordinate(x,y);
    }
    else if (propertyType == "text") {
        return value["value"].GetString();
    }
    else if (propertyType == "vector_coordinate") {
        std::vector<lc::geo::Coordinate> coords;
        for (const auto& coord : value["value"].GetArray()) {
            double x = coord["x"].GetDouble();
            double y = coord["y"].GetDouble();
            coords.push_back(lc::geo::Coordinate(x, y));
        }
        return coords;
    }
    else {
        return (double)-1;
    }
}

void CopyManager::createEntity(const std::string& entityName, const lc::entity::PropertiesMap& propertiesList) {
    if (_cadMdiChild != nullptr) {
        std::shared_ptr<lc::operation::EntityBuilder> entityBuilder = std::make_shared<lc::operation::EntityBuilder>(_cadMdiChild->document());
        lc::entity::CADEntity_CSPtr newEntity = nullptr;

        if (entityName == "Line") {
            newEntity = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(), lc::geo::Coordinate(), _cadMdiChild->activeLayer());
        }
        else if (entityName == "Point") {
            newEntity = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(), _cadMdiChild->activeLayer());
        }
        else if (entityName == "Circle") {
            newEntity = std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(), 1, _cadMdiChild->activeLayer());
        }
        else if (entityName == "Arc") {
            newEntity = std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(), 1, 1, 1, false, _cadMdiChild->activeLayer());
        }
        else if (entityName == "Ellipse") {
            newEntity = std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(), lc::geo::Coordinate(), 1, 1, 1, false, _cadMdiChild->activeLayer());
        }
        else if (entityName == "Spline") {
            newEntity = std::make_shared<lc::entity::Spline>(std::vector<geo::Coordinate>(),std::vector<double>(), std::vector<geo::Coordinate>(),1, false, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, lc::geo::Spline::splineflag::CLOSED, _cadMdiChild->activeLayer());
        }
        else if (entityName == "DimAligned") {
            newEntity = std::make_shared<lc::entity::DimAligned>(lc::geo::Coordinate(), lc::geo::Coordinate(), lc::TextConst::AttachmentPoint::Middle_center, 1, 1, lc::TextConst::LineSpacingStyle::AtLeast, "", lc::geo::Coordinate(), lc::geo::Coordinate(), _cadMdiChild->activeLayer());
        }
        else if (entityName == "DimAngular") {
            newEntity = std::make_shared<lc::entity::DimAngular>(lc::geo::Coordinate(), lc::geo::Coordinate(), lc::TextConst::AttachmentPoint::Middle_center, 1, 1, lc::TextConst::LineSpacingStyle::AtLeast, "", lc::geo::Coordinate(), lc::geo::Coordinate(), lc::geo::Coordinate(), lc::geo::Coordinate(), _cadMdiChild->activeLayer());
        }
        else if (entityName == "DimDiametric") {
            newEntity = std::make_shared<lc::entity::DimDiametric>(lc::geo::Coordinate(), lc::geo::Coordinate(), lc::TextConst::AttachmentPoint::Middle_center, 1, 1, lc::TextConst::LineSpacingStyle::AtLeast, "", lc::geo::Coordinate(), 1, _cadMdiChild->activeLayer());
        }
        else if (entityName == "DimLinear") {
            newEntity = std::make_shared<lc::entity::DimLinear>(lc::geo::Coordinate(), lc::geo::Coordinate(), lc::TextConst::AttachmentPoint::Middle_center, 1, 1, lc::TextConst::LineSpacingStyle::AtLeast, "", lc::geo::Coordinate(), lc::geo::Coordinate(), 1, 1, _cadMdiChild->activeLayer());
        }
        else if (entityName == "DimRadial") {
            newEntity = std::make_shared<lc::entity::DimRadial>(lc::geo::Coordinate(), lc::geo::Coordinate(), lc::TextConst::AttachmentPoint::Middle_center, 1, 1, lc::TextConst::LineSpacingStyle::AtLeast, "", lc::geo::Coordinate(), 1, _cadMdiChild->activeLayer());
        }
        else if (entityName == "LWPolyline") {
            newEntity = std::make_shared<lc::entity::LWPolyline>(std::vector<lc::entity::LWVertex2D>(), 1, 1, 1, false, lc::geo::Coordinate(), _cadMdiChild->activeLayer());
        }

        if (newEntity != nullptr) {
            newEntity = newEntity->setProperties(propertiesList);
            entityBuilder->appendEntity(newEntity);
            entityBuilder->execute();
        }
    }
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
