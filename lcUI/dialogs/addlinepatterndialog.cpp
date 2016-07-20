#include "addlinepatterndialog.h"
#include "ui_addlinepatterndialog.h"

AddLinePatternDialog::AddLinePatternDialog(QWidget *parent) :
    AddLinePatternDialog(nullptr, parent) {
}

AddLinePatternDialog::AddLinePatternDialog(lc::DxfLinePattern_CSPtr linePattern, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddLinePatternDialog),
    _linePattern(linePattern) {

    ui->setupUi(this);

    _layout = static_cast<QVBoxLayout*>(ui->pathList->layout());
    if(!_layout) {
        throw "Unable to cast AddLinePatternDialog pathList layout to QVBoxLayout";
    }

    connect(ui->newValueButton, &QPushButton::pressed, this, &AddLinePatternDialog::onNewValuePressed);

    if(linePattern != nullptr) {
        ui->name->setText(linePattern->name().c_str());
        ui->description->setText(linePattern->description().c_str());

        for (auto value : linePattern->path()) {
            auto pathPart = new LinePatternPathPart(value, this);
            connect(pathPart, &LinePatternPathPart::update, this, &AddLinePatternDialog::generatePreview);

            _layout->insertWidget(_layout->count() - 1, pathPart);
        }
    }
}

void AddLinePatternDialog::generatePreview() {
    std::string name = ui->name->text().toStdString();
    if(name.empty()) {
        name = "tmp";
    }

    std::vector<double> path;
    double length;

    auto parts = findChildren<LinePatternPathPart*>();

    for(auto part : parts) {
        auto type = part->type();
        auto value = part->value();

        switch(type) {
            case LinePatternPathPart::PATH_DOT:
                path.push_back(0);
                break;

            case LinePatternPathPart::PATH_PLAIN:
                path.push_back(value);
                length += value;
                break;

            case LinePatternPathPart::PATH_SPACE:
                path.push_back(-value);
                length += value;
                break;
        }
    }

    _linePattern = std::make_shared<lc::DxfLinePattern>(
            name,
            ui->description->text().toStdString(),
            path,
            length);

    QPixmap previewPixmap(ui->preview->frameSize());

    LinePatternPainter painter(_linePattern, &previewPixmap);
    painter.drawLinePattern();

    ui->preview->setPixmap(previewPixmap);
}


void AddLinePatternDialog::onNewValuePressed() {
    auto pathPart = new LinePatternPathPart(this);
    connect(pathPart, &LinePatternPathPart::update, this, &AddLinePatternDialog::generatePreview);

    _layout->insertWidget(_layout->count() - 1, pathPart);
}

lc::DxfLinePattern_CSPtr AddLinePatternDialog::linePattern() {
    return _linePattern;
}