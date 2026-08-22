#include "comboboxgui.h"
#include "ui_comboboxgui.h"

using namespace lc::ui::api;

ComboBoxGUI::ComboBoxGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    ui(new Ui::ComboBoxGUI)
{
    ui->setupUi(this);
    _type = "combobox";
    setCopyPasteEnabled(false);

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _comboBox = qobject_cast<QComboBox*>(ui->horizontalLayout->itemAt(1)->widget());
    ui->horizontalLayout->insertStretch(1);

    _textLabel->setText(QString(this->label().c_str()));

    connect(_comboBox, QOverload<int>::of(&QComboBox::activated), this, &ComboBoxGUI::activatedCallbacks);
}

ComboBoxGUI::~ComboBoxGUI()
{
    delete ui;
}

void ComboBoxGUI::removeStretch() {
    ui->horizontalLayout->removeItem(ui->horizontalLayout->itemAt(1));
}

void ComboBoxGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _textLabel->setText(QString(newLabel.c_str()));
}

void ComboBoxGUI::addCallback(lc::scripting::ScriptCallback cb) {
    _activatedCallbacks.push_back(std::move(cb));
}

void ComboBoxGUI::activatedCallbacks(int /*index*/) {
    // Phase 4 PR-5a — neutral callback invocation.
    const std::string current = _comboBox->currentText().toStdString();
    for (auto& cb : _activatedCallbacks) {
        cb.call(current);
    }
}

void ComboBoxGUI::addItem(const std::string& item, int index) {
    if (index == -1) {
        _comboBox->addItem(QString(item.c_str()));
    }
    else {
        _comboBox->insertItem(index, QString(item.c_str()));
    }
}

std::string ComboBoxGUI::value() const {
    return _comboBox->currentText().toStdString();
}

void ComboBoxGUI::setValue(const std::string& val) {
    int found = _comboBox->findText(val.c_str());
    if (found != -1) {
        setValue(found);
    }
}

void ComboBoxGUI::setValue(int index) {
    _comboBox->setCurrentIndex(index);
}

void ComboBoxGUI::getValue(lc::scripting::Map& map) {
    (*map)[_key] = lc::scripting::ScriptValue(value());
}

void ComboBoxGUI::copyValue(QDataStream& stream) {
}

void ComboBoxGUI::pasteValue(QDataStream& stream) {
}

void ComboBoxGUI::hideLabel() {
    _textLabel->hide();
}
