#include "numbergui.h"
#include "ui_numbergui.h"

using namespace lc::ui::api;

NumberGUI::NumberGUI(std::string label, double minVal, double maxVal, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::NumberGUI)
{
    ui->setupUi(this);

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _spinBox = qobject_cast<QDoubleSpinBox*>(ui->horizontalLayout->itemAt(1)->widget());
    ui->horizontalLayout->insertStretch(1);

    _spinBox->setMinimum(minVal);
    _spinBox->setMaximum(maxVal);

    connect(_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &NumberGUI::valueChangedCallbacks);

    _textLabel->setText(QString(this->label().c_str()));
}

NumberGUI::~NumberGUI()
{
    delete ui;
}

void NumberGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void NumberGUI::valueChangedCallbacks(double val) {
    for (kaguya::LuaRef& cb : _callbacks) {
        cb(val);
    }
}

void NumberGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _textLabel->setText(QString(newLabel.c_str()));
}

double NumberGUI::value() const {
    return _spinBox->value();
}

void NumberGUI::setValue(double val) {
    _spinBox->setValue(val);
}

void NumberGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = value();
}
