#include "textdialog.h"
#include "ui_textdialog.h"

using namespace lc::ui::dialog;

TextDialog::TextDialog(lc::ui::MainWindow* mainWindowIn, QWidget* parent)
    : QDialog(parent),
    _mainWindow(mainWindowIn),
    ui(new Ui::TextDialog)
{
    ui->setupUi(this);

    textEdit = qobject_cast<QTextEdit*>(ui->verticalLayout->itemAt(0)->widget());
    drawingDirectionComboBox = qobject_cast<QComboBox*>(ui->horizontalLayout->itemAt(1)->widget());
    halignComboBox = qobject_cast<QComboBox*>(ui->horizontalLayout_2->itemAt(1)->widget());
    valignComboBox = qobject_cast<QComboBox*>(ui->horizontalLayout_3->itemAt(1)->widget());
    heightSpinBox = qobject_cast<QDoubleSpinBox*>(ui->horizontalLayout_4->itemAt(1)->widget());
    angleSpinBox = qobject_cast<QDoubleSpinBox*>(ui->horizontalLayout_5->itemAt(1)->widget());

    drawingDirectionComboBox->addItem("None");
    drawingDirectionComboBox->addItem("Backward");
    drawingDirectionComboBox->addItem("Upside Down");

    halignComboBox->addItem("HALeft");
    halignComboBox->addItem("HARight");
    halignComboBox->addItem("HAAligned");
    halignComboBox->addItem("HAMiddle");
    halignComboBox->addItem("HAFit");

    valignComboBox->addItem("VABaseline");
    valignComboBox->addItem("VABottom");
    valignComboBox->addItem("VAMiddle");
    valignComboBox->addItem("VATop");

    QPushButton* okButton = qobject_cast<QPushButton*>(ui->horizontalLayout_6->itemAt(0)->widget());
    QPushButton* cancelButton = qobject_cast<QPushButton*>(ui->horizontalLayout_6->itemAt(1)->widget());
    connect(okButton, &QPushButton::released, this, &TextDialog::okButtonClicked);
    connect(cancelButton, &QPushButton::released, this, &TextDialog::cancelButtonClicked);
}

TextDialog::~TextDialog()
{
    delete ui;
}

void TextDialog::okButtonClicked() {
    kaguya::State state(_mainWindow->luaInterface()->luaState());
    _mainWindow->runOperation(state["TextOperations"]);
    this->close();
}

void TextDialog::cancelButtonClicked() {
    this->close();
}
