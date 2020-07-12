#include "uisettings.h"

#include <rapidjson/document.h>
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
    settingsDocument.ParseStream(isw);
    customizeToolbar->readData(settingsDocument);

    settingsFile.close();
}
