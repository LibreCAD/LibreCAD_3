#include "uisettings.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

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

    customizeToolbar->generateData(&streamWriter);

    streamWriter.writeEndDocument();
    settingsFile.close();
}

void UiSettings::readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings) {
    QString fileName = defaultSettings ? defaultSettingsFileName : settingsFileName;
    QFile settingsFile(fileName);

    if (!settingsFile.open(QIODevice::ReadOnly)) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    QXmlStreamReader streamReader(&settingsFile);
    customizeToolbar->readData(&streamReader);
    settingsFile.close();
}
