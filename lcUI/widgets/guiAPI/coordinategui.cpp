#include "coordinategui.h"
#include "ui_coordinategui.h"

#include <QDoubleValidator>

using namespace lc::ui::api;

CoordinateGUI::CoordinateGUI(std::string label, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::CoordinateGUI)
{
    ui->setupUi(this);

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _xcoordEdit = qobject_cast<QLineEdit*>(ui->horizontalLayout->itemAt(2)->widget());
    _ycoordEdit = qobject_cast<QLineEdit*>(ui->horizontalLayout->itemAt(4)->widget());
    ui->horizontalLayout->insertStretch(1);

    QDoubleValidator* doubleValidator = new QDoubleValidator(this);
    doubleValidator->setDecimals(3);

    _textLabel->setText(QString(this->label().c_str()));
    _xcoordEdit->setValidator(doubleValidator);
    _ycoordEdit->setValidator(doubleValidator);

    _xcoordEdit->setText(0);
    _ycoordEdit->setText(0);

    connect(_xcoordEdit, &QLineEdit::editingFinished, this, &CoordinateGUI::editingFinishedCallbacks);
    connect(_ycoordEdit, &QLineEdit::editingFinished, this, &CoordinateGUI::editingFinishedCallbacks);
    connect(_xcoordEdit, &QLineEdit::textChanged, this, &CoordinateGUI::textChangedCallbacks);
    connect(_ycoordEdit, &QLineEdit::textChanged, this, &CoordinateGUI::textChangedCallbacks);
}

CoordinateGUI::~CoordinateGUI() {
    delete ui;
}

void CoordinateGUI::updateCoordinate() {
    _coordinate = lc::geo::Coordinate(_xcoordEdit->text().toDouble(), _ycoordEdit->text().toDouble());
}

void CoordinateGUI::updateCoordinateDisplay() {
    this->setToolTip(generateTooltip());
}

QString CoordinateGUI::generateTooltip() const {
    QString xcoordText = _xcoordEdit->text();
    QString ycoordText = _ycoordEdit->text();
    if (xcoordText == "") {
        xcoordText = "0";
    }
    if (ycoordText == "") {
        ycoordText = "0";
    }
    QString tooltipcoord = QString("(%1,%2)").arg(xcoordText, ycoordText);
    QString tooltipmagnitude = QString("Magnitude - %1").arg(QString::number(_coordinate.magnitude()));
    QString tooltipangle = QString("Angle - %1").arg(QString::number(_coordinate.angle() * 180 / 3.141592));
    return tooltipcoord + "\n" + tooltipmagnitude + "\n" + tooltipangle;
}

void CoordinateGUI::addFinishCallback(kaguya::LuaRef cb) {
    _callbacks_finished.push_back(cb);
}

void CoordinateGUI::addOnChangeCallback(kaguya::LuaRef cb) {
    _callbacks_onchange.push_back(cb);
}

void CoordinateGUI::editingFinishedCallbacks() {
    updateCoordinate();

    for (kaguya::LuaRef& cb : _callbacks_finished) {
        cb(_coordinate);
    }
}

void CoordinateGUI::textChangedCallbacks() {
    updateCoordinate();
    updateCoordinateDisplay();

    for (kaguya::LuaRef& cb : _callbacks_onchange) {
        cb(_coordinate);
    }
}

void CoordinateGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _textLabel->setText(QString(newLabel.c_str()));
}

lc::geo::Coordinate CoordinateGUI::value() const {
    return _coordinate;
}

void CoordinateGUI::setValue(lc::geo::Coordinate coord) {
    _xcoordEdit->setText(QString::number(coord.x()));
    _ycoordEdit->setText(QString::number(coord.y()));
}

void CoordinateGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = value();
}
