#include "lcadviewer.h"
#include "lcadviewerplugin.h"

#include <QtCore/QtPlugin>

LCADViewerPlugin::LCADViewerPlugin(QObject* parent)
    : QObject(parent) {
    m_initialized = false;
}

void LCADViewerPlugin::initialize(QDesignerFormEditorInterface* /* core */) {
    if (m_initialized) {
        return;
    }

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool LCADViewerPlugin::isInitialized() const {
    return m_initialized;
}

QWidget* LCADViewerPlugin::createWidget(QWidget* parent) {
    return new LCADViewer(parent);
}

QString LCADViewerPlugin::name() const {
    return QLatin1String("LCADViewer");
}

QString LCADViewerPlugin::group() const {
    return QLatin1String("");
}

QIcon LCADViewerPlugin::icon() const {
    return QIcon();
}

QString LCADViewerPlugin::toolTip() const {
    return QLatin1String("LibreCAD viewer");
}

QString LCADViewerPlugin::whatsThis() const {
    return QLatin1String("");
}

bool LCADViewerPlugin::isContainer() const {
    return false;
}

QString LCADViewerPlugin::domXml() const {
    return QLatin1String("<widget class=\"LCADViewer\" name=\"lCADViewer\">\n</widget>\n");
}

QString LCADViewerPlugin::includeFile() const {
    return QLatin1String("lcadviewer.h");
}

Q_EXPORT_PLUGIN2(lcadviewerplugin, LCADViewerPlugin)
