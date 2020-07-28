#include "textgui.h"
#include "ui_textgui.h"

#include <QString>

#include <iostream>

using namespace lc::ui::api;

TextGUI::TextGUI(std::string label, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::TextGUI)
{
    ui->setupUi(this);
    _type = "text";

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _lineEdit = qobject_cast<QLineEdit*>(ui->horizontalLayout->itemAt(1)->widget());
    ui->horizontalLayout->insertStretch(1);

    _textLabel->setText(QString(this->label().c_str()));

    connect(_lineEdit, &QLineEdit::editingFinished, this, &TextGUI::editingFinishedCallbacks);
    connect(_lineEdit, &QLineEdit::textChanged, this, &TextGUI::textChangedCallbacks);
}

TextGUI::~TextGUI()
{
    delete ui;
}

void TextGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _textLabel->setText(QString(newLabel.c_str()));
}

std::string TextGUI::value() const {
    return _lineEdit->text().toStdString();
}

void TextGUI::setValue (const std::string& newText) {
    _lineEdit->setText(QString(newText.c_str()));
}

void TextGUI::addFinishCallback(kaguya::LuaRef cb) {
    _callbacks_finished.push_back(cb);
}

void TextGUI::addOnChangeCallback(kaguya::LuaRef cb) {
    _callbacks_onchange.push_back(cb);
}

void TextGUI::editingFinishedCallbacks() {
    for (kaguya::LuaRef& cb : _callbacks_finished) {
        cb();
    }
}

void TextGUI::textChangedCallbacks(const QString& changedText) {
    for (kaguya::LuaRef& cb : _callbacks_onchange) {
        cb(changedText.toStdString());
    }
}

void TextGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = value();
}

void TextGUI::copyValue(QDataStream& stream) {
    stream << QString(value().c_str());
}

void TextGUI::pasteValue(QDataStream& stream) {
    QString val;
    stream >> val;
    setValue(val.toStdString());
    editingFinishedCallbacks();
}
