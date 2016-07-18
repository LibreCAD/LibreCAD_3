#include "addlayerdialog.h"
#include "ui_addlayerdialog.h"

AddLayerDialog::AddLayerDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AddLayerDialog){

    ui->setupUi(this);
}

AddLayerDialog::~AddLayerDialog() {
    delete ui;
}

void AddLayerDialog::accept() {
    if(ui->name->text().isEmpty()) {
        QToolTip::showText(ui->name->mapToGlobal(QPoint()), tr("Name cannot be empty."));
        return;
    }

    auto layer = std::make_shared<const lc::Layer>(
            ui->name->text().toStdString(),
            lc::MetaLineWidthByValue(ui->width->value()),
            lc::Color(ui->r->value(),
                      ui->g->value(),
                      ui->b->value(),
                      ui->a->value()
            )
    );

    emit newLayer(layer);

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