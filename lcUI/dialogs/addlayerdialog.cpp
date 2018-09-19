#include <QtWidgets/QMessageBox>
#include "addlayerdialog.h"
#include "ui_addlayerdialog.h"

AddLayerDialog::AddLayerDialog(lc::Document_SPtr document, QWidget* parent) :
    AddLayerDialog(nullptr, std::move(document), parent) {
}

AddLayerDialog::AddLayerDialog(lc::Layer_CSPtr oldLayer, lc::Document_SPtr document, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::AddLayerDialog),
        _oldLayer(std::move(oldLayer)) {

    ui->setupUi(this);

    linePatternSelect = new lc::ui::LinePatternSelect(std::move(document), this, false, false);
    lineWidthSelect = new lc::ui::LineWidthSelect(nullptr, this, false, false);
    colorSelect = new lc::ui::ColorSelect(nullptr, this, false, false);

    auto layout = dynamic_cast<QFormLayout*>(this->layout());
    if(layout != nullptr) {
        layout->setWidget(1, QFormLayout::FieldRole, colorSelect);
        layout->setWidget(2, QFormLayout::FieldRole, lineWidthSelect);
        layout->setWidget(3, QFormLayout::FieldRole, linePatternSelect);
    }

    if(_oldLayer != nullptr) {
        ui->name->setText(_oldLayer->name().c_str());
        if(_oldLayer->linePattern() != nullptr) {
            linePatternSelect->setCurrentText(_oldLayer->linePattern()->name().c_str());
        }
        lineWidthSelect->setWidth(std::make_shared<lc::MetaLineWidthByValue>(_oldLayer->lineWidth()));
        colorSelect->setColor(_oldLayer->color());

        if(_oldLayer->linePattern() != nullptr) {
            int linePatternIndex = linePatternSelect->findText(_oldLayer->linePattern()->name().c_str());

            if (linePatternIndex != -1) {
                linePatternSelect->setCurrentIndex(linePatternIndex);
            }
        }
    }
}

AddLayerDialog::~AddLayerDialog() {
    delete ui;
}

void AddLayerDialog::accept() {
    if(ui->name->text().isEmpty()) {
        QToolTip::showText(ui->name->mapToGlobal(QPoint()), tr("Name cannot be empty."));
        return;
    }

    auto linePattern = std::dynamic_pointer_cast<const lc::DxfLinePatternByValue>(linePatternSelect->linePattern());
    lc::Layer_CSPtr layer;

    auto lineWidth = std::dynamic_pointer_cast<const lc::MetaLineWidthByValue>(lineWidthSelect->lineWidth());
    if(lineWidth == nullptr) {
        return;
    }

    if(linePattern == nullptr) {
        layer = std::make_shared<const lc::Layer>(
                ui->name->text().toStdString(),
                *lineWidth,
                colorSelect->color()
        );
    }
    else {
        layer = std::make_shared<const lc::Layer>(
                ui->name->text().toStdString(),
                *lineWidth,
                colorSelect->color(),
                linePattern,
                false
        );
    }

    if(_oldLayer == nullptr) {
        emit newLayer(layer);
    }
    else {
        emit editLayer(_oldLayer, layer);
    }

    this->close();
}