#include "uisettings.h"

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

#include <iostream>

using namespace lc::ui;

void UiSettings::writeSettings(widgets::CustomizeToolbar* customizeToolbar) {
    std::ofstream  settingsFile(settingsFileName);

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
    std::ifstream settingsFile(fileName);

    while(settingsFile.fail()){
        if (fileName == settingsFileName) {
            std::cout << "No settings file found, loading default settings." << std::endl;
            fileName = defaultSettingsFileName;
            settingsFile.open(fileName);
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
    std::ifstream schemaFile(schemaFileName);

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
