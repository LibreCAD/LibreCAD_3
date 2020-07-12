#include "uisettings.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

#include <iostream>

using namespace lc::ui;

void UiSettings::writeSettings(widgets::CustomizeToolbar* customizeToolbar) {
    QFile settingsFile(settingsFileName);

    if (!settingsFile.open(QIODevice::WriteOnly)) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    QXmlStreamWriter streamWriter(&settingsFile);

    streamWriter.setAutoFormatting(true);
    streamWriter.writeStartDocument();

    std::ofstream ofs("ui_settings.json");

    if (ofs.fail()) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    customizeToolbar->generateData(&streamWriter);
    //customizeToolbar->generateDataJSON(writer);

    streamWriter.writeEndDocument();
    settingsFile.close();
    ofs.close();
}

void UiSettings::readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings) {
    QString fileName = defaultSettings ? defaultSettingsFileName : settingsFileName;
    QFile settingsFile(fileName);

    if (!settingsFile.open(QIODevice::ReadOnly)) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    std::ifstream infile("ui_settings.json");

    if(infile.fail()){
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    QXmlStreamReader streamReader(&settingsFile);

    rapidjson::IStreamWrapper isw(infile);

    rapidjson::Document settingsDocument;
    settingsDocument.ParseStream(isw);

    //customizeToolbar->readData(&streamReader);
    customizeToolbar->readDataJSON(settingsDocument);

    infile.close();
    settingsFile.close();
}
