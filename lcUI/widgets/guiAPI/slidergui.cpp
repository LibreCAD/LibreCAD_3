#include "slidergui.h"
#include "ui_slidergui.h"

using namespace lc::ui::api;

SliderGUI::SliderGUI(std::string label, int setMin, int setMax, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::SliderGUI)
{
    ui->setupUi(this);
    _type = "slider";

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _slider = qobject_cast<QSlider*>(ui->horizontalLayout->itemAt(1)->widget());
    _displayValue = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(2)->widget());
    ui->horizontalLayout->insertStretch(1);

    _textLabel->setText(QString(this->label().c_str()));
    _slider->setMinimum(setMin);
    _slider->setMaximum(setMax);

    connect(_slider, &QSlider::valueChanged, this, &SliderGUI::valueChangedCallbacks);
    connect(_slider, &QSlider::valueChanged, this, &SliderGUI::updateValueDisplay);
}

SliderGUI::~SliderGUI() {
    delete ui;
}

void SliderGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _textLabel->setText(QString(newLabel.c_str()));
}

void SliderGUI::addCallback(kaguya::LuaRef cb) {
    _valueChangeCallbacks.push_back(cb);
}

void SliderGUI::valueChangedCallbacks(int value) {
    for (kaguya::LuaRef& cb : _valueChangeCallbacks) {
        cb(value);
    }
}

void SliderGUI::updateValueDisplay(int value) {
    _displayValue->setText(QString::number(value));
}

int SliderGUI::value() const {
    return _slider->value();
}

void SliderGUI::setValue(int val) {
    _slider->setValue(val);
}

void SliderGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = value();
}

void SliderGUI::copyValue(QDataStream& stream) {
    stream << value();
}

void SliderGUI::pasteValue(QDataStream& stream) {
    int val;
    stream >> val;
    setValue(val);
}
