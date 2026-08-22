#include "anglegui.h"
#include "ui_anglegui.h"

#include <QDoubleValidator>

using namespace lc::ui::api;

AngleGUI::AngleGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    angleType(AngleGUI::AngleType::Degrees),
    ui(new Ui::AngleGUI)
{
    ui->setupUi(this);
    ui->horizontalLayout->insertStretch(1);

    _type = "angle";
    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _lineEdit = qobject_cast<QLineEdit*>(ui->horizontalLayout->itemAt(2)->widget());
    _angleTypeButton = qobject_cast<QPushButton*>(ui->horizontalLayout->itemAt(3)->widget());

    _angleTypeButton->setStyleSheet("padding-left: 1px; padding-right: 1px;");
    _textLabel->setText(QString(this->label().c_str()));

    QDoubleValidator* doubleValidator = new QDoubleValidator(this);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    doubleValidator->setDecimals(3);
    _lineEdit->setValidator(doubleValidator);

    QFontMetrics fm = _lineEdit->fontMetrics();
    _lineEdit->setFixedWidth(fm.averageCharWidth() * 17);

    connect(_angleTypeButton, &QPushButton::clicked, this, &AngleGUI::toggleUnit);
    connect(_lineEdit, &QLineEdit::editingFinished, this, &AngleGUI::editingFinishedCallbacks);
    connect(_lineEdit, &QLineEdit::textChanged, this, &AngleGUI::textChangedCallbacks);
}

AngleGUI::~AngleGUI() {
    delete ui;
}

void AngleGUI::toDegrees() {
    if (angleType == AngleGUI::AngleType::Degrees) {
        return;
    }

    angleType = AngleGUI::AngleType::Degrees;
    _angleTypeButton->setText("Deg");
    double angleInRad = _lineEdit->text().toDouble();
    double angleInDeg = angleInRad * 180 / 3.1416;

    _lineEdit->setText(QString::number(angleInDeg));
}

void AngleGUI::toRadians() {
    if (angleType == AngleGUI::AngleType::Radians) {
        return;
    }

    angleType = AngleGUI::AngleType::Radians;
    _angleTypeButton->setText("Rad");
    double angleInDeg = _lineEdit->text().toDouble();
    double angleInRad = angleInDeg * 3.1416 / 180;

    _lineEdit->setText(QString::number(angleInRad));
}

void AngleGUI::toggleUnit() {
    if (angleType == AngleGUI::AngleType::Degrees) {
        toRadians();
    }
    else if (angleType == AngleGUI::AngleType::Radians) {
        toDegrees();
    }
}

void AngleGUI::editingFinishedCallbacks() {
    double angleInRad;
    // always call callbacks with angle in radians
    if (angleType == AngleGUI::AngleType::Degrees) {
        double angleInDeg = _lineEdit->text().toDouble();
        angleInRad = angleInDeg * 3.1416 / 180;
    }
    else {
        angleInRad = _lineEdit->text().toDouble();
    }

    // Phase 4 PR-5a — neutral callback invocation.
    for (auto& cb : _callbacks_finished) {
        cb.call(angleInRad);
    }
}

void AngleGUI::textChangedCallbacks(const QString& changedText) {
    double angleInRad;
    if (angleType == AngleGUI::AngleType::Degrees) {
        double angleInDeg = changedText.toDouble();
        angleInRad = angleInDeg * 3.1416 / 180;
    }
    else {
        angleInRad = changedText.toDouble();
    }

    for (auto& cb : _callbacks_onchange) {
        cb.call(angleInRad);
    }
}

void AngleGUI::addFinishCallback(lc::scripting::ScriptCallback cb) {
    _callbacks_finished.push_back(std::move(cb));
}

void AngleGUI::addOnChangeCallback(lc::scripting::ScriptCallback cb) {
    _callbacks_onchange.push_back(std::move(cb));
}

void AngleGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _textLabel->setText(QString(newLabel.c_str()));
}

double AngleGUI::value() const {
    double angleInRad;
    QString angleText = _lineEdit->text();
    if (angleType == AngleGUI::AngleType::Degrees) {
        double angleInDeg = angleText.toDouble();
        angleInRad = angleInDeg * 3.1416 / 180;
    }
    else {
        angleInRad = angleText.toDouble();
    }
    return angleInRad;
}

void AngleGUI::setValue(double val) {
    double angle;
    if (angleType == AngleGUI::AngleType::Degrees) {
        angle = val * 180 / 3.1416;
    }
    else {
        angle = val;
    }
    _lineEdit->setText(QString::number(angle));
}

void AngleGUI::getValue(lc::scripting::Map& map) {
    (*map)[_key] = lc::scripting::ScriptValue(value());
}

void AngleGUI::copyValue(QDataStream& stream) {
    stream << value();
}

void AngleGUI::pasteValue(QDataStream& stream) {
    double val;
    stream >> val;
    setValue(val);
    editingFinishedCallbacks();
}

void AngleGUI::hideLabel() {
    _textLabel->hide();
}
