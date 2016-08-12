#include "addlayerdialog.h"
#include "ui_addlayerdialog.h"

AddLayerDialog::AddLayerDialog(lc::Document_SPtr document, QWidget* parent) :
    AddLayerDialog(nullptr, document, parent) {
}

AddLayerDialog::AddLayerDialog(lc::Layer_CSPtr oldLayer, lc::Document_SPtr document, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::AddLayerDialog),
        _oldLayer(oldLayer) {

    ui->setupUi(this);

    linePatternSelect = new LinePatternSelect(document, this);
    lineWidthSelect = new LineWidthSelect(this);
    colorSelect = new ColorSelect(this);

    auto layout = dynamic_cast<QFormLayout*>(this->layout());
    if(layout) {
        layout->setWidget(1, QFormLayout::FieldRole, colorSelect);
        layout->setWidget(2, QFormLayout::FieldRole, lineWidthSelect);
        layout->setWidget(3, QFormLayout::FieldRole, linePatternSelect);
    }

    if(oldLayer != nullptr) {
        ui->name->setText(oldLayer->name().c_str());
        if(oldLayer->linePattern() != nullptr) {
            linePatternSelect->setCurrentText(oldLayer->linePattern()->name().c_str());
        }
        lineWidthSelect->setWidth(oldLayer->lineWidth().width());
        colorSelect->setColor(oldLayer->color());

        if(oldLayer->linePattern() != nullptr) {
            int linePatternIndex = linePatternSelect->findText(oldLayer->linePattern()->name().c_str());

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

    auto linePattern = linePatternSelect->linePattern();
    lc::Layer_CSPtr layer;

    if(linePattern == nullptr) {
        layer = std::make_shared<const lc::Layer>(
                ui->name->text().toStdString(),
                *lineWidthSelect->lineWidth(),
                colorSelect->color()
        );
    }
    else {
        layer = std::make_shared<const lc::Layer>(
                ui->name->text().toStdString(),
                *lineWidthSelect->lineWidth(),
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