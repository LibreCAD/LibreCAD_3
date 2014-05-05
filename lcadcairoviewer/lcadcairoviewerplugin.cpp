#include "LCADCairoViewer.h"
#include "LCADCairoViewerplugin.h"

#include <QtCore/QtPlugin>

LCADCairoViewerPlugin::LCADCairoViewerPlugin(QObject* parent)
    : QObject(parent) {
    m_initialized = false;
}

void LCADCairoViewerPlugin::initialize(QDesignerFormEditorInterface* /* core */) {
    if (m_initialized) {
        return;
    }

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool LCADCairoViewerPlugin::isInitialized() const {
    return m_initialized;
}

QWidget* LCADCairoViewerPlugin::createWidget(QWidget* parent) {
    return new LCADCairoViewer(parent);
}

QString LCADCairoViewerPlugin::name() const {
    return QLatin1String("LCADCairoViewer");
}

QString LCADCairoViewerPlugin::group() const {
    return QLatin1String("");
}

QIcon LCADCairoViewerPlugin::icon() const {
    return QIcon();
}

QString LCADCairoViewerPlugin::toolTip() const {
    return QLatin1String("LibreCAD viewer");
}

QString LCADCairoViewerPlugin::whatsThis() const {
    return QLatin1String("");
}

bool LCADCairoViewerPlugin::isContainer() const {
    return false;
}

QString LCADCairoViewerPlugin::domXml() const {
    return QLatin1String("<widget class=\"LCADCairoViewer\" name=\"LCADCairoViewer\">\n</widget>\n");
}

QString LCADCairoViewerPlugin::includeFile() const {
    return QLatin1String("LCADCairoViewer.h");
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(LCADCairoViewerplugin, LCADCairoViewerPlugin)
#endif
