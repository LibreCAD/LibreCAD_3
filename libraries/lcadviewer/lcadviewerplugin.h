#ifndef LCADVIEWERPLUGIN_H
#define LCADVIEWERPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class LCADViewerPlugin : public QObject, public QDesignerCustomWidgetInterface {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface" FILE "CustomWidget.json")
        Q_INTERFACES(QDesignerCustomWidgetInterface)

    public:
        LCADViewerPlugin(QObject* parent = 0);

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
