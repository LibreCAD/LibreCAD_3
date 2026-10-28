#pragma once

#include <QTest>
#include <lcUI/widgets/layers.h>
#include <ui_layers.h>
#include <cadmdichild.h>

using lc::ui::widgets::Layers;
using lc::ui::widgets::LayerModel;

class LayersTest : public lc::ui::widgets::Layers {
public:
    LayersTest(lc::ui::CadMdiChild* mdiChild = nullptr, QWidget* parent = nullptr) :
        Layers(mdiChild, parent)
    {}

    unsigned int nbLayers() {
        QAbstractItemModel* model = ui->layerList->model();
        return model->rowCount();
    }

    void selectLayer(unsigned int row) {
        ui->layerList->selectRow(row);
    }

    void cellClick(unsigned int col, unsigned int row) {
        int xPos = ui->layerList->columnViewportPosition(col) + 2;
        int yPos = ui->layerList->rowViewportPosition(row) + 2;

        QWidget* viewport = ui->layerList->viewport();

        QTest::mouseClick(viewport, Qt::LeftButton, Qt::NoModifier, QPoint(xPos, yPos));
    }

    void addLayer(lc::meta::Layer_CSPtr layer) {
        createLayer(layer);
    }

    void remove() {
        QTest::mouseClick(ui->deleteButton, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5), 100);
    }

    void newClicked() {
        QTest::mouseClick(ui->newButton, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5), 100);
    }
};
