#include "uisettings.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

using namespace lc::ui;

void UiSettings::writeSettings(widgets::CustomizeToolbar* customizeToolbar) {
    QFile settingsFile(settingsFileName);
    settingsFile.open(QIODevice::WriteOnly);
    QXmlStreamWriter streamWriter(&settingsFile);

    streamWriter.setAutoFormatting(true);
    streamWriter.writeStartDocument();

    customizeToolbar->generateData(&streamWriter);

    streamWriter.writeEndDocument();
    settingsFile.close();
}
