#include "uisettings.h"

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <QCoreApplication>

#include <iostream>


using namespace lc::ui;

void UiSettings::writeSettings(widgets::CustomizeToolbar* customizeToolbar) {
    rapidjson::Document settingsDocument = getSettingsDocument(settingsFileName);
    if (settingsDocument.HasMember("toolbar")) {
        settingsDocument.RemoveMember("toolbar");
    }

    customizeToolbar->generateData(settingsDocument);
    writeSettingsFile(settingsDocument);
}

void UiSettings::readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings) {
    std::string fileName = defaultSettings ? defaultSettingsFileName : settingsFileName;
    rapidjson::Document settingsDocument = getSettingsDocument(fileName);

    customizeToolbar->readData(settingsDocument);
}

bool UiSettings::validateSettingsDocument(rapidjson::Document& inputDocument) {
    std::ifstream schemaFile(QCoreApplication::applicationDirPath().toStdString() + _filePaths["settings_load"] + schemaFileName);

    if (schemaFile.fail()) {
        std::cout << "Schema file not found" << std::endl;
        return true;
    }

    rapidjson::IStreamWrapper isw(schemaFile);
    rapidjson::Document schemaDocument;

    if (schemaDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Schema file is invalid, not json format." << std::endl;
        return true;
    }

    rapidjson::SchemaDocument schema(schemaDocument);
    rapidjson::SchemaValidator validator(schema);

    if (!inputDocument.Accept(validator)) {
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        printf("Invalid schema: %s\n", sb.GetString());
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());

        return false;
    }

    return true;
}

void UiSettings::writeDockSettings(const std::map<std::string, int>& positions, std::map<std::string, int>& proportions) {
    rapidjson::Document settingsDocument = getSettingsDocument(settingsFileName);
    rapidjson::Value dockPositions(rapidjson::kArrayType);

    for (auto iter = positions.begin(); iter != positions.end(); ++iter) {
        rapidjson::Value posValue(rapidjson::kObjectType);
        rapidjson::Value widgetname;
        widgetname.SetString(iter->first.c_str(), iter->first.size(), settingsDocument.GetAllocator());
        posValue.AddMember("widget", widgetname, settingsDocument.GetAllocator());
        posValue.AddMember("position", rapidjson::Value(iter->second), settingsDocument.GetAllocator());

        if (proportions.find(iter->first) != proportions.end()) {
            posValue.AddMember("proportion", rapidjson::Value(proportions[iter->first]), settingsDocument.GetAllocator());
        }

        dockPositions.PushBack(posValue, settingsDocument.GetAllocator());
    }

    if (settingsDocument.HasMember("dockPositions")) {
        settingsDocument.RemoveMember("dockPositions");
    }

    settingsDocument.AddMember("dockPositions", dockPositions, settingsDocument.GetAllocator());
    writeSettingsFile(settingsDocument);
}

std::map<std::string, int> UiSettings::readDockSettings(std::map<std::string, int>& proportions) {
    rapidjson::Document settingsDocument = getSettingsDocument(settingsFileName);
    std::map<std::string, int> positions;

    if (settingsDocument.HasMember("dockPositions")) {
        rapidjson::Value dockPositions = settingsDocument["dockPositions"].GetArray();

        for (rapidjson::Value::ConstValueIterator iter = dockPositions.Begin(); iter != dockPositions.End(); ++iter) {
            if (iter->HasMember("widget") && iter->HasMember("position")) {
                std::string widgetName = (*iter)["widget"].GetString();
                int pos = (*iter)["position"].GetInt();
                positions[widgetName] = pos;

                if (iter->HasMember("proportion")) {
                    int prop = (*iter)["proportion"].GetInt();
                    proportions[widgetName] = prop;
                }
            }
        }
    }

    return positions;
}

rapidjson::Document UiSettings::getSettingsDocument(std::string fileName) {
    std::ifstream settingsFile(QCoreApplication::applicationDirPath().toStdString()  + _filePaths["settings_load"] + fileName);

    while (settingsFile.fail()) {
        if (fileName == settingsFileName) {
            std::cout << "No settings file found, loading default settings." << std::endl;
            fileName = defaultSettingsFileName;
            settingsFile.open(QCoreApplication::applicationDirPath().toStdString() + _filePaths["settings_load"] + fileName);
        }
        else {
            std::cout << "Default settings not found" << std::endl;
            return rapidjson::Document();;
        }
    }

    rapidjson::IStreamWrapper isw(settingsFile);
    rapidjson::Document settingsDocument;

    if (settingsDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Error with settings document, not in json format" << std::endl;
        return rapidjson::Document();
    }

    if (!validateSettingsDocument(settingsDocument)) {
        settingsFile.close();
        return rapidjson::Document();
    }

    settingsFile.close();
    return std::move(settingsDocument);
}

void UiSettings::writeSettingsFile(rapidjson::Document& document) {
    std::ofstream outFile(QCoreApplication::applicationDirPath().toStdString() + _filePaths["settings_load"] + settingsFileName);

    if (outFile.fail()) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    rapidjson::OStreamWrapper osw(outFile);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    document.Accept(writer);
    outFile.close();
}
