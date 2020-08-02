#include "inputgui.h"

#include <QMimeData>
#include <QClipboard>
#include <QApplication>
#include <QMenu>

using namespace lc::ui::api;

InputGUI::InputGUI(std::string _label, QWidget* parent)
    :
    QWidget(parent),
    _label(_label),
    _copyPasteEnabled(true)
{}

std::string InputGUI::label() const {
    return _label;
}

void InputGUI::setLabel(const std::string& newLabel) {
    _label = newLabel;
}

void InputGUI::setKey(const std::string& keyIn) {
    _key = keyIn;
}

const std::string InputGUI::key() const {
    return _key;
}

void InputGUI::copyEvent() {
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << QString(_type.c_str());
    this->copyValue(stream);

    mimeData->setData("application/x-propertydata", encodedData);
    QApplication::clipboard()->setMimeData(mimeData);
}

void InputGUI::pasteEvent() {
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    QByteArray itemData = mimeData->data("application/x-propertydata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QString guiType;
    dataStream >> guiType;
    if (guiType.toStdString() == _type) {
        this->pasteValue(dataStream);
    }
}

void InputGUI::contextMenuEvent(QContextMenuEvent* event) {
    if (_copyPasteEnabled) {
        QMenu menu(this);
        QAction* copyAction = new QAction("Copy");
        connect(copyAction, &QAction::triggered, this, &InputGUI::copyEvent);
        menu.addAction(copyAction);

        const QMimeData* mimeData = QApplication::clipboard()->mimeData();
        QByteArray itemData = mimeData->data("application/x-propertydata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QString guiType;
        dataStream >> guiType;

        // Only show paste option if selection of same type
        if (guiType.toStdString() == _type) {
            QAction* pasteAction = new QAction("Paste");
            menu.addAction(pasteAction);
            connect(pasteAction, &QAction::triggered, this, &InputGUI::pasteEvent);
        }

        menu.exec(event->globalPos());
    }
}

void InputGUI::setCopyPasteEnabled(bool enable) {
    _copyPasteEnabled = enable;
}
