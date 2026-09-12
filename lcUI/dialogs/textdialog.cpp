#include "textdialog.h"
#include "ui_textdialog.h"

// Phase 4 PR-9c — okButtonClicked routes through the neutral scripting
// layer: MTextOperations resolved via runOperationByName (which walks
// the PR-7 resolver list, so phase-5 Python would find the same class
// too); copyEntity invoked via currentOperation().callMethod, killing
// the `updateTextOp` dostring.
#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptvalue.h>

using namespace lc::ui::dialog;

TextDialog::TextDialog(lc::ui::MainWindow* mainWindowIn, QWidget* parent)
    : QDialog(parent),
    ui(new Ui::TextDialog),
    _mainWindow(mainWindowIn)
{
    ui->setupUi(this);

    const std::vector<std::pair<QString, QString>> symbolData = {
        {"Diameter(⌀)", "⌀"},
        {"Degree(°)", "°"},
        {"Plus / Minus(±)", "±"},
        {"At(@)", "@"},
        {"Hash(#)", "#"},
        {"Dollar($)", "$"},
        {"Copyright(©)", "©"},
        {"Registered(®)", "®"},
        {"Paragraph(§)", "§"},
        {"Pi(π)", "π"},
        {"Pound(£)", "£"},
        {"Yen(¥)", "¥"},
        {"Times(×)", "×"},
        {"Division(÷)", "÷"}
    };

    textEdit = qobject_cast<QTextEdit*>(ui->verticalLayout->itemAt(0)->widget());
    fontComboBox = qobject_cast<QComboBox*>(ui->horizontalLayout_1->itemAt(1)->widget());
    drawingDirectionComboBox = qobject_cast<QComboBox*>(ui->horizontalLayout->itemAt(1)->widget());
    heightSpinBox = qobject_cast<QDoubleSpinBox*>(ui->horizontalLayout_4->itemAt(1)->widget());
    angleSpinBox = qobject_cast<QDoubleSpinBox*>(ui->horizontalLayout_5->itemAt(1)->widget());
    underlineCheckBox = qobject_cast<QCheckBox*>(ui->horizontalLayout_9->itemAt(0)->widget());
    strikethroughCheckBox = qobject_cast<QCheckBox*>(ui->horizontalLayout_9->itemAt(1)->widget());
    boldCheckbox = qobject_cast<QCheckBox*>(ui->horizontalLayout_10->itemAt(0)->widget());
    italicCheckbox = qobject_cast<QCheckBox*>(ui->horizontalLayout_10->itemAt(1)->widget());

    connect(underlineCheckBox, &QCheckBox::toggled, this, &TextDialog::underlineToggled);
    connect(strikethroughCheckBox, &QCheckBox::toggled, this, &TextDialog::strikethroughToggled);
    connect(boldCheckbox, &QCheckBox::toggled, this, &TextDialog::boldToggled);
    connect(italicCheckbox, &QCheckBox::toggled, this, &TextDialog::italicToggled);

    drawingDirectionComboBox->addItem("None", lc::TextConst::DrawingDirection::None);
    drawingDirectionComboBox->addItem("Backward", lc::TextConst::DrawingDirection::Backward);
    drawingDirectionComboBox->addItem("Upside Down", lc::TextConst::DrawingDirection::UpsideDown);

    std::vector<std::string> fontList = _mainWindow->cadMdiChild()->viewer()->docCanvas()->getFontList();
    for (const std::string& font : fontList) {
        fontComboBox->addItem(QString(font.c_str()), QString(font.c_str()));
    }

    heightSpinBox->setMaximum(100000);
    heightSpinBox->setValue(100);
    angleSpinBox->setMaximum(360);
    angleSpinBox->setWrapping(true);
    angleSpinBox->setValue(0);

    QComboBox* symbolComboBox = qobject_cast<QComboBox*>(ui->verticalLayout_8->itemAt(0)->widget());
    for (const std::pair<QString, QString>& symbol : symbolData) {
        symbolComboBox->addItem(symbol.first, symbol.second);
    }

    alignmentGroupBox = qobject_cast<QGroupBox*>(ui->horizontalLayout_8->itemAt(0)->widget());
    int numAlignments = alignmentGroupBox->layout()->count();

    for (int i = 0; i < numAlignments; i++) {
        QToolButton* toolButton = qobject_cast<QToolButton*>(alignmentGroupBox->layout()->itemAt(i)->widget());
        toolButton->setAutoExclusive(true);
        connect(toolButton, &QToolButton::toggled, this, &TextDialog::alignmentToggled);

        if (numAlignments / 2 == i) {
            toolButton->setChecked(true);
        }
    }

    QPushButton* okButton = qobject_cast<QPushButton*>(ui->horizontalLayout_6->itemAt(0)->widget());
    QPushButton* cancelButton = qobject_cast<QPushButton*>(ui->horizontalLayout_6->itemAt(1)->widget());
    QPushButton* insertSymbolButton = qobject_cast<QPushButton*>(ui->verticalLayout_8->itemAt(1)->widget());
    connect(okButton, &QPushButton::released, this, &TextDialog::okButtonClicked);
    connect(cancelButton, &QPushButton::released, this, &TextDialog::cancelButtonClicked);
    connect(insertSymbolButton, &QPushButton::released, this, &TextDialog::insertSymbolClicked);
}

TextDialog::~TextDialog()
{
    delete ui;
}

void TextDialog::okButtonClicked() {
    // Phase 4 PR-9c — MTextOperations resolution routes through
    // MainWindow::runOperationByName, which walks the PR-7 resolver
    // list (Lua-globals resolver today; phase 5 will push a Python
    // registry resolver AFTER it — same class name in either source
    // will resolve, satisfying the two-source design).
    _mainWindow->runOperationByName("MTextOperations");

    lc::builder::MTextBuilder textBuilder;
    textBuilder.setLayer(_mainWindow->cadMdiChild()->activeLayer());
    textBuilder.setMetaInfo(_mainWindow->cadMdiChild()->metaInfoManager()->metaInfo());
    textBuilder.setBlock(_mainWindow->cadMdiChild()->activeViewport());

    textBuilder.setTextValue(textEdit->toPlainText().toStdString());
    textBuilder.setTextFont(fontComboBox->itemData(fontComboBox->currentIndex()).toString().toStdString());
    textBuilder.setHeight(heightSpinBox->value());
    textBuilder.setAngle(angleSpinBox->value() * 3.1416/180);
    textBuilder.setDrawingDirection((lc::TextConst::DrawingDirection)drawingDirectionComboBox->itemData(drawingDirectionComboBox->currentIndex()).toInt());
    textBuilder.setHorizontalAlign(halign);
    textBuilder.setVerticalAlign(valign);
    textBuilder.setUnderlined(underlineCheckBox->isChecked());
    textBuilder.setStrikethrough(strikethroughCheckBox->isChecked());
    textBuilder.setBold(boldCheckbox->isChecked());
    textBuilder.setItalic(italicCheckbox->isChecked());

    // Phase 4 PR-9c — replaces `updateTextOp = function(textEntity)
    // mainWindow:currentOperation():copyEntity(textEntity) end` dostring
    // codegen.  The current operation (just started above by
    // runOperationByName) receives the freshly-built MText via its
    // `copyEntity` method — LuaObjectImpl::callMethod prepends `self`
    // so the Lua-side `op:copyEntity(t)` invocation shape is preserved.
    lc::scripting::ScriptObject curOp = _mainWindow->currentOperation();
    if (!curOp.isNil()) {
        std::vector<lc::scripting::ScriptValue> args;
        args.emplace_back(textBuilder.build());
        curOp.callMethod("copyEntity", args);
    }

    this->close();
}

void TextDialog::cancelButtonClicked() {
    this->close();
}

void TextDialog::insertSymbolClicked() {
    QComboBox* symbolComboBox = qobject_cast<QComboBox*>(ui->verticalLayout_8->itemAt(0)->widget());

    if (symbolComboBox != nullptr) {
        textEdit->insertPlainText(symbolComboBox->itemData(symbolComboBox->currentIndex()).toString());
    }
}

void TextDialog::alignmentToggled(bool toggle) {
    if (toggle) {
        int numAlignments = alignmentGroupBox->layout()->count();
        for (int i = 0; i < numAlignments; i++) {
            QToolButton* toolButton = qobject_cast<QToolButton*>(alignmentGroupBox->layout()->itemAt(i)->widget());
            if (toolButton->isChecked()) {
                if (toolButton->toolTip() == "Top Right") {
                    setAlignment(3);
                }
                else if (toolButton->toolTip() == "Top Left") {
                    setAlignment(1);
                }
                else if (toolButton->toolTip() == "Top Center") {
                    setAlignment(2);
                }
                else if (toolButton->toolTip() == "Bottom Right") {
                    setAlignment(9);
                }
                else if (toolButton->toolTip() == "Bottom Left") {
                    setAlignment(7);
                }
                else if (toolButton->toolTip() == "Bottom Center") {
                    setAlignment(8);
                }
                else if (toolButton->toolTip() == "Middle Right") {
                    setAlignment(6);
                }
                else if (toolButton->toolTip() == "Middle Left") {
                    setAlignment(4);
                }
                else if (toolButton->toolTip() == "Middle Center") {
                    setAlignment(5);
                }

                return;
            }
        }
    }
}

void TextDialog::setAlignment(int i) {
    switch (i % 3) {
    default:
    case 1:
        halign = lc::TextConst::HAlign::HALeft;
        break;
    case 2:
        halign = lc::TextConst::HAlign::HACenter;
        break;
    case 0:
        halign = lc::TextConst::HAlign::HARight;
        break;
    }

    switch ((int)(std::ceil(i / 3.0))) {
    default:
    case 1:
        valign = lc::TextConst::VAlign::VATop;
        break;
    case 2:
        valign = lc::TextConst::VAlign::VAMiddle;
        break;
    case 3:
        valign = lc::TextConst::VAlign::VABottom;
        break;
    }
}

void TextDialog::underlineToggled(bool toggle) {
    auto f = textEdit->font();
    f.setUnderline(toggle);
    textEdit->setFont(f);
}

void TextDialog::strikethroughToggled(bool toggle) {
    auto f = textEdit->font();
    f.setStrikeOut(toggle);
    textEdit->setFont(f);
}

void TextDialog::boldToggled(bool toggle) {
    auto f = textEdit->font();
    f.setBold(toggle);
    textEdit->setFont(f);
}

void TextDialog::italicToggled(bool toggle) {
    auto f = textEdit->font();
    f.setItalic(toggle);
    textEdit->setFont(f);
}
