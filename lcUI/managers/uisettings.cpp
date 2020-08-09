#include "uisettings.h"

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

#include <iostream>

using namespace lc::ui;

void UiSettings::writeSettings(widgets::CustomizeToolbar* customizeToolbar) {
    std::ofstream  settingsFile(_filePaths["settings_load"] + settingsFileName);

    if (settingsFile.fail()) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    rapidjson::OStreamWrapper osw(settingsFile);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    customizeToolbar->generateData(writer);

    settingsFile.close();
}

void UiSettings::readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings) {
    std::string fileName = defaultSettings ? defaultSettingsFileName : settingsFileName;
    std::ifstream settingsFile(_filePaths["settings_load"] + fileName);

    while(settingsFile.fail()){
        if (fileName == settingsFileName) {
            std::cout << "No settings file found, loading default settings." << std::endl;
            fileName = defaultSettingsFileName;
            settingsFile.open(_filePaths["settings_load"] + fileName);
        }else{
            std::cout << "Default settings not found" << std::endl;
            return;
        }
    }

    rapidjson::IStreamWrapper isw(settingsFile);
    rapidjson::Document settingsDocument;

    if (settingsDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Erro with settings document, not in json format" << std::endl;
        return;
    }

    if (!validateSettingsDocument(settingsDocument)) {
        settingsFile.close();
        return;
    }

    customizeToolbar->readData(settingsDocument);

    settingsFile.close();
}

bool UiSettings::validateSettingsDocument(rapidjson::Document& inputDocument) {
    std::ifstream schemaFile(_filePaths["settings_load"] + schemaFileName);

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

void UiSettings::writeDockSettings(int layerp, int clip, int toolp, int propertyp) {
    std::string fileName = settingsFileName;
    std::ifstream settingsFile(_filePaths["settings_load"] + fileName);

    while (settingsFile.fail()) {
        if (fileName == settingsFileName) {
            std::cout << "No settings file found, loading default settings." << std::endl;
            fileName = defaultSettingsFileName;
            settingsFile.open(_filePaths["settings_load"] + fileName);
        }
        else {
            std::cout << "Default settings not found" << std::endl;
            return;
        }
    }

    rapidjson::IStreamWrapper isw(settingsFile);
    rapidjson::Document settingsDocument;

    if (settingsDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Erro with settings document, not in json format" << std::endl;
        return;
    }

    if (!validateSettingsDocument(settingsDocument)) {
        settingsFile.close();
        return;
    }

    std::ofstream outFile(_filePaths["settings_load"] + "testfile.json");

    if (outFile.fail()) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    rapidjson::OStreamWrapper osw(outFile);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    rapidjson::Value dockPositions(rapidjson::kArrayType);
    /* Write Dock Settings  */
    std::map<std::string, int> positions = {
        {"Toolbar", toolp},
        {"CliCommand", clip},
        {"Layers", layerp},
        {"PropertyEditor", propertyp}
    };

    for (auto iter = positions.begin(); iter != positions.end(); ++iter) {
        rapidjson::Value posValue(rapidjson::kObjectType);
        rapidjson::Value widgetname;
        widgetname.SetString(iter->first.c_str(), iter->first.size(), settingsDocument.GetAllocator());
        posValue.AddMember("widget", widgetname, settingsDocument.GetAllocator());
        posValue.AddMember("position", rapidjson::Value(iter->second), settingsDocument.GetAllocator());
        dockPositions.PushBack(posValue, settingsDocument.GetAllocator());
    }

    settingsDocument.AddMember("dockPositions", dockPositions, settingsDocument.GetAllocator());
    settingsDocument.Accept(writer);

    outFile.close();
    settingsFile.close();
}

std::map<std::string, int> UiSettings::readDockSettings() {
    std::string fileName = "testfile.json";
    std::ifstream settingsFile(_filePaths["settings_load"] + fileName);

    while (settingsFile.fail()) {
        if (fileName == settingsFileName) {
            std::cout << "No settings file found, loading default settings." << std::endl;
            fileName = defaultSettingsFileName;
            settingsFile.open(_filePaths["settings_load"] + fileName);
        }
        else {
            std::cout << "Default settings not found" << std::endl;
            return std::map<std::string, int>();
        }
    }

    rapidjson::IStreamWrapper isw(settingsFile);
    rapidjson::Document settingsDocument;

    if (settingsDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Erro with settings document, not in json format" << std::endl;
        return std::map<std::string, int>();
    }

    if (!validateSettingsDocument(settingsDocument)) {
        settingsFile.close();
        return std::map<std::string, int>();
    }

    std::map<std::string, int> result;

    if (settingsDocument.HasMember("dockPositions")) {
        rapidjson::Value dockPositions = settingsDocument["dockPositions"].GetArray();

        for (rapidjson::Value::ConstValueIterator iter = dockPositions.Begin(); iter != dockPositions.End(); ++iter) {
            if (iter->HasMember("widget") && iter->HasMember("position")) {
                std::string widgetName = (*iter)["widget"].GetString();
                int pos = (*iter)["position"].GetInt();
                result[widgetName] = pos;
            }
        }
    }

    return result;
}
