#include "addlinepatterndialog.h"
#include "ui_addlinepatterndialog.h"

using namespace lc::ui::dialog;

AddLinePatternDialog::AddLinePatternDialog(lc::storage::Document_SPtr document, QWidget *parent) :
    AddLinePatternDialog(std::move(document), nullptr, parent) {
}

AddLinePatternDialog::AddLinePatternDialog(lc::storage::Document_SPtr document, lc::meta::DxfLinePatternByValue_CSPtr linePattern, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddLinePatternDialog),
    _document(std::move(document)),
    _oldLinePattern(std::move(linePattern)) {

    ui->setupUi(this);

    _layout = dynamic_cast<QVBoxLayout*>(ui->pathList->layout());
    if(_layout == nullptr) {
        throw std::runtime_error("Unable to cast AddLinePatternDialog pathList layout to QVBoxLayout");
    }

    if(_oldLinePattern != nullptr) {
        ui->name->setText(_oldLinePattern->name().c_str());
        ui->description->setText(_oldLinePattern->description().c_str());

        for (auto value : _oldLinePattern->path()) {
            auto pathPart = new widgets::LinePatternPathPart(value, this);
            connect(pathPart, &widgets::LinePatternPathPart::update, this, &AddLinePatternDialog::generatePreview);

            _layout->insertWidget(_layout->count() - 1, pathPart);
        }

        ui->saveButton->setText(SAVE_AS_NEW_TEXT);

        editButton = new QPushButton(EDIT_TEXT);
        connect(editButton, &QPushButton::pressed, this, &AddLinePatternDialog::onEditButtonPressed);
        ui->buttonLayout->addWidget(editButton);
    }
}

void AddLinePatternDialog::generatePreview() {
    std::string name = ui->name->text().toStdString();
    if(name.empty()) {
        name = "tmp";
    }

    std::vector<double> path;
    double length = 0.0;

    auto parts = findChildren<widgets::LinePatternPathPart*>();

    for(auto part : parts) {
        auto type = part->type();
        auto value = part->value();

        switch(type) {
            case widgets::LinePatternPathPart::PATH_DOT:
                path.push_back(0);
                break;

            case widgets::LinePatternPathPart::PATH_PLAIN:
                path.push_back(value);
                length += value;
                break;

            case widgets::LinePatternPathPart::PATH_SPACE:
                path.push_back(-value);
                length += value;
                break;
        }
    }

    _linePattern = std::make_shared<lc::meta::DxfLinePatternByValue>(
            name,
            ui->description->text().toStdString(),
            path,
            length);

    QPixmap previewPixmap(ui->preview->frameSize());

    widgets::LinePatternPainter painter(&previewPixmap, _linePattern);
    painter.drawLinePattern();

    ui->preview->setPixmap(previewPixmap);
}


void AddLinePatternDialog::on_newValueButton_pressed() {
    auto pathPart = new widgets::LinePatternPathPart(this);
    connect(pathPart, &widgets::LinePatternPathPart::update, this, &AddLinePatternDialog::generatePreview);

    _layout->insertWidget(_layout->count() - 1, pathPart);
}

void AddLinePatternDialog::on_cancelButton_pressed() {
    this->close();
}

void AddLinePatternDialog::on_saveButton_pressed() {
    auto operation = std::make_shared<lc::operation::AddLinePattern>(_document, _linePattern);
    operation->execute();

    this->close();
}

void AddLinePatternDialog::onEditButtonPressed() {
    auto operation = std::make_shared<lc::operation::ReplaceLinePattern>(_document, _oldLinePattern, _linePattern);
    operation->execute();

    this->close();
}
