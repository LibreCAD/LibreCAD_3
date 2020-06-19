#include "coordinategui.h"
#include "ui_coordinategui.h"

#include <QTimer>
#include <QDoubleValidator>

using namespace lc::ui::api;

CoordinateGUI::CoordinateGUI(std::string label, QWidget* parent)
    : 
    InputGUI(label, parent),
    _pointSelectionEnabled(false),
    ui(new Ui::CoordinateGUI)
{
    ui->setupUi(this);

    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _xcoordEdit = qobject_cast<QLineEdit*>(ui->horizontalLayout->itemAt(2)->widget());
    _ycoordEdit = qobject_cast<QLineEdit*>(ui->horizontalLayout->itemAt(4)->widget());
    _pointButton = qobject_cast<QPushButton*>(ui->horizontalLayout->itemAt(5)->widget());
    ui->horizontalLayout->insertStretch(1);
    _pointButton->setStyleSheet("padding-left: 1px; padding-right: 1px;");

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
    connect(_pointButton, &QPushButton::toggled, this, &CoordinateGUI::togglePointSelection);
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

void CoordinateGUI::enableCoordinateSelection(lc::ui::MainWindow* mainWindow) {
    this->mainWindow = mainWindow;
    if (mainWindow != nullptr) {
        connect(mainWindow, &lc::ui::MainWindow::point, this, &CoordinateGUI::pointSelected);
    }
}

void CoordinateGUI::pointSelected(lc::geo::Coordinate point) {
    if (_pointSelectionEnabled) {
        this->setValue(point);
        parentWidget()->activateWindow();
        this->setFocus();
        _pointButton->toggled(false);
        _pointButton->setChecked(false);
    }
}

void CoordinateGUI::togglePointSelection(bool toggle) {
    _pointSelectionEnabled = toggle;
    if (toggle) {
        mainWindow->activateWindow();
    }
}
