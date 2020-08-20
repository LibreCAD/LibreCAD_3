#include "textdialog.h"
#include "ui_textdialog.h"

using namespace lc::ui::dialog;

TextDialog::TextDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TextDialog)
{
    ui->setupUi(this);

    drawingDirectionComboBox = ui->horizontalLayout->findChild<QComboBox*>("comboBox");
    halignComboBox = ui->horizontalLayout_2->findChild<QComboBox*>("comboBox_2");
    valignComboBox = ui->horizontalLayout_3->findChild<QComboBox*>("comboBox_3");
    heightSpinBox = ui->horizontalLayout_4->findChild<QDoubleSpinBox*>("doubleSpinBox");
    angleSpinBox = ui->horizontalLayout_5->findChild<QDoubleSpinBox*>("doubleSpinBox_2");
}

TextDialog::~TextDialog()
{
    delete ui;
}
