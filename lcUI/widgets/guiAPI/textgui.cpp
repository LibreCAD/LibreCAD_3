#include "textgui.h"
#include "ui_textgui.h"

#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QString>

#include <iostream>

using namespace lc::ui::api;

TextGUI::TextGUI(std::string label, QWidget* parent)
    : 
    InputGUI(label, parent),
    ui(new Ui::TextGUI)
{
    ui->setupUi(this);

    QLabel* textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    textLabel->setText(QString(this->label().c_str()));
}

TextGUI::~TextGUI()
{
    delete ui;
}

void TextGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    QLabel* textLabel = qobject_cast<QLabel*>(ui->horizontalLayout->itemAt(0)->widget());
    textLabel->setText(QString(newLabel.c_str()));
}
