#ifndef LCADCAIROVIEWERPLUGIN_H
#define LCADCAIROVIEWERPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class LCADCairoViewerPlugin : public QObject, public QDesignerCustomWidgetInterface {
        Q_OBJECT
#if QT_VERSION > 0x050000
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface" FILE "CustomWidget.json")
#endif
        Q_INTERFACES(QDesignerCustomWidgetInterface)

    public:
        LCADCairoViewerPlugin(QObject* parent = 0);

        bool isContainer() const;
        bool isInitialized() const;
        QIcon icon() const;
        QString domXml() const;
        QString group() const;
        QString includeFile() const;
        QString name() const;
        QString toolTip() const;
        QString whatsThis() const;
        QWidget* createWidget(QWidget* parent);
        void initialize(QDesignerFormEditorInterface* core);

    private:
        bool m_initialized;
};

#endif
