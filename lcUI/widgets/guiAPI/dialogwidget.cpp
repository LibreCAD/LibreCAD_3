#include "dialogwidget.h"
#include "ui_dialogwidget.h"

#include <iostream>
#include <QGridLayout>

using namespace lc::ui::api;

DialogWidget::DialogWidget(QWidget* parent)
    : 
    QDialog(parent),
    ui(new Ui::DialogWidget)
{
    ui->setupUi(this);
    show();

    auto gridLayout = qobject_cast<QGridLayout*>(this->layout());
    vboxlayout = gridLayout->findChild<QVBoxLayout*>("verticalLayout");
}

DialogWidget::~DialogWidget()
{
    delete ui;
}

void DialogWidget::addWidget(InputGUI* guiWidget) {
    guiWidget->setParent(this);
    vboxlayout->addWidget(guiWidget);
}
