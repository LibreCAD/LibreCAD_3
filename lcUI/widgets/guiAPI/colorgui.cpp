#include "colorgui.h"
#include "ui_colorgui.h"

#include <QColorDialog>

using namespace lc::ui::api;

ColorGUI::ColorGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    _color("white"),
    ui(new Ui::ColorGUI)
{
    ui->setupUi(this);
    
    _type = "color";
    _textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    _colorButton = qobject_cast<QPushButton*>(ui->horizontalLayout->itemAt(1)->widget());
    ui->horizontalLayout->insertStretch(1);

    _textLabel->setText(QString(this->label().c_str()));

    const QString COLOR_STYLE("QPushButton { background-color : %1; color : %2; }");
    _colorButton->setStyleSheet(COLOR_STYLE.arg(_color.name()).arg("black"));

    connect(_colorButton, &QPushButton::clicked, this, &ColorGUI::changeColor);
}

ColorGUI::~ColorGUI() {
    delete ui;
}

void ColorGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = value();
}

void ColorGUI::changeColor() {
    _color = QColorDialog::getColor();
    QColor textColor = getIdealTextColor(_color);

    const QString COLOR_STYLE("QPushButton { background-color : %1; color : %2; }");
    _colorButton->setStyleSheet(COLOR_STYLE.arg(_color.name()).arg(textColor.name()));

    const QString COLOR_LABEL("R:%1 G:%2 B:%3");
    _colorButton->setText(COLOR_LABEL.arg(QString::number(_color.red()), 3, QLatin1Char('0')).arg(QString::number(_color.green()), 3, QLatin1Char('0')).arg(QString::number(_color.blue()), 3, QLatin1Char('0')));

    colorSelectedCallbacks();
}

QColor ColorGUI::getIdealTextColor(const QColor& rBackgroundColor) const
{
    const int THRESHOLD = 105;
    int BackgroundDelta = (rBackgroundColor.red() * 0.299) + (rBackgroundColor.green() * 0.587) + (rBackgroundColor.blue() * 0.114);
    return QColor((255 - BackgroundDelta < THRESHOLD) ? Qt::black : Qt::white);
}

lc::Color ColorGUI::value() const {
    return lc::Color(_color.red(), _color.green(), _color.blue(), _color.alpha());
}

void ColorGUI::setValue(lc::Color col) {
    _color = QColor(col.red() * 255, col.green() * 255, col.blue() * 255, col.alpha());
    QColor textColor = getIdealTextColor(_color);

    const QString COLOR_STYLE("QPushButton { background-color : %1; color : %2; }");
    _colorButton->setStyleSheet(COLOR_STYLE.arg(_color.name()).arg(textColor.name()));

    const QString COLOR_LABEL("R:%1 G:%2 B:%3");
    _colorButton->setText(COLOR_LABEL.arg(QString::number(_color.red()), 3, QLatin1Char('0')).arg(QString::number(_color.green()), 3, QLatin1Char('0')).arg(QString::number(_color.blue()), 3, QLatin1Char('0')));

    colorSelectedCallbacks();
}

void ColorGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void ColorGUI::colorSelectedCallbacks() {
    for (kaguya::LuaRef& cb : _callbacks) {
        cb(value());
    }
}

void ColorGUI::copyValue(QDataStream& stream) {
    double r, g, b, a;
    lc::Color val = value();
    r = val.red();
    g = val.green();
    b = val.blue();
    a = val.alpha();
    stream << r << g << b << a;
}

void ColorGUI::pasteValue(QDataStream& stream) {
    double r, g, b, a;
    stream >> r >> g >> b >> a;
    lc::Color col(r, g, b, a);
    setValue(col);
    colorSelectedCallbacks();
}
