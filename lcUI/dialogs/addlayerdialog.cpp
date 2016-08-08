#include "addlayerdialog.h"
#include "ui_addlayerdialog.h"

AddLayerDialog::AddLayerDialog(lc::Document_SPtr document, QWidget* parent) :
    AddLayerDialog(nullptr, document, parent) {
}

AddLayerDialog::AddLayerDialog(lc::Layer_CSPtr oldLayer, lc::Document_SPtr document, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::AddLayerDialog),
        _document(document),
        _oldLayer(oldLayer) {

    ui->setupUi(this);

    linePatternSelect = new LinePatternSelect(document, this);
    lineWidthSelect = new LineWidthSelect(this);

    auto layout = dynamic_cast<QFormLayout*>(this->layout());
    if(layout) {
        layout->setWidget(2, QFormLayout::FieldRole, lineWidthSelect);
        layout->setWidget(3, QFormLayout::FieldRole, linePatternSelect);
    }

    if(oldLayer != nullptr) {
        ui->name->setText(oldLayer->name().c_str());

        ui->r->setValue(oldLayer->color().redI());
        ui->g->setValue(oldLayer->color().greenI());
        ui->b->setValue(oldLayer->color().blueI());
        ui->a->setValue(oldLayer->color().alphaI());

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
                lc::Color(ui->r->value(),
                          ui->g->value(),
                          ui->b->value(),
                          ui->a->value()
                )
        );
    }
    else {
        layer = std::make_shared<const lc::Layer>(
                ui->name->text().toStdString(),
                *lineWidthSelect->lineWidth(),
                lc::Color(ui->r->value(),
                          ui->g->value(),
                          ui->b->value(),
                          ui->a->value()
                ),
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

void AddLayerDialog::on_pickColorButton_clicked() {
    QColor currentColor(ui->r->value(),
                        ui->g->value(),
                        ui->b->value(),
                        ui->a->value());
    auto colorDialog = new QColorDialog();

    colorDialog->setCurrentColor(currentColor);
    colorDialog->show();

    connect(colorDialog, &QColorDialog::colorSelected, this, &AddLayerDialog::on_colorChanged);
}

void AddLayerDialog::on_colorChanged(const QColor& color) {
    ui->r->setValue(color.red());
    ui->g->setValue(color.green());
    ui->b->setValue(color.blue());
    ui->a->setValue(color.alpha());
}