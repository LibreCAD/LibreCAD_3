#include <cadmdichild.h>
#include "layers.h"
#include "ui_layers.h"

Layers::Layers(QMdiArea* mdiArea, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Layers) {

    ui->setupUi(this);

    model = new LayerModel(this);

    ui->layerList->setModel(model);
    ui->layerList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->layerList->horizontalHeader()->setSectionResizeMode(LayerModel::NAME, QHeaderView::Stretch);

    onSubwindowActivated(mdiArea->activeSubWindow());

    connect(mdiArea, &QMdiArea::subWindowActivated, this, &Layers::onSubwindowActivated);
    connect(model, &LayerModel::nameChanged, this, &Layers::changeLayerName);
}

Layers::~Layers() {
    delete ui;
}

lc::Layer_CSPtr Layers::activeLayer() {
    QModelIndexList selection = ui->layerList->selectionModel()->selectedRows();

    if(selection.isEmpty()) {
        if(model->rowCount() == 0) {
            createLayer(std::make_shared<lc::Layer>(
                    "0",
                    lc::Color(255, 255, 255, 255)
            ));
        }

        ui->layerList->selectRow(0);
        return model->layerAt(0);
    }

    return model->layerAt(selection.first().row());
}

void Layers::on_newButton_clicked() {
    auto dialog = new AddLayerDialog(document->linePatterns(), this);
    dialog->show();

    connect(dialog, &AddLayerDialog::newLayer, this, &Layers::createLayer);
}

void Layers::on_deleteButton_clicked() {
    QModelIndexList selection = ui->layerList->selectionModel()->selectedRows();

    deleteLayer(model->layerAt(selection.first().row()));
}

void Layers::onSubwindowActivated(QMdiSubWindow *window) {
    if(document != nullptr) {
        document->addLayerEvent().disconnect<Layers, &Layers::on_addLayerEvent>(this);
        document->removeLayerEvent().disconnect<Layers, &Layers::on_removeLayerEvent>(this);
        document->replaceLayerEvent().disconnect<Layers, &Layers::on_replaceLayerEvent>(this);
    }

    if(window != 0) {
        auto cadMdiChild = static_cast<CadMdiChild*>(window->widget());
        document = cadMdiChild->document();

        document->addLayerEvent().connect<Layers, &Layers::on_addLayerEvent>(this);
        document->removeLayerEvent().connect<Layers, &Layers::on_removeLayerEvent>(this);
        document->replaceLayerEvent().connect<Layers, &Layers::on_replaceLayerEvent>(this);

        updateLayerList();
    }
    else {
        document = nullptr;
    }
}

void Layers::on_layerList_clicked(const QModelIndex& index) {
    if(!index.isValid()) {
        return;
    }

    auto layer = model->layerAt(index.row());

    if(index.column() != LayerModel::NAME) {
        auto locked = layer->isFrozen();

        switch(index.column()) {
            case LayerModel::LOCKED:
                locked = !locked;
                break;
        }

        auto newLayer = std::make_shared<const lc::Layer>(
                layer->name(),
                layer->lineWidth(),
                layer->color(),
                layer->linePattern(),
                locked
        );

        replaceLayer(layer, newLayer);
    }
}

void Layers::onSelectionChanged(const QItemSelection& selection, const QItemSelection&) {

}

void Layers::changeLayerName(lc::Layer_CSPtr& layer, const std::string& name) {
    if(name.empty()) {
        return;
    }

    auto newLayer = std::make_shared<const lc::Layer>(
        name,
        layer->lineWidth(),
        layer->color(),
        layer->linePattern(),
        layer->isFrozen()
    );

    replaceLayer(layer, newLayer);
}

void Layers::createLayer(lc::Layer_CSPtr layer) {
    if(document != nullptr) {
        auto operation = std::make_shared<lc::operation::AddLayer>(document, layer);
        operation->execute();
    }
}

void Layers::deleteLayer(lc::Layer_CSPtr layer) {
    if(document != nullptr) {
        auto operation = std::make_shared<lc::operation::RemoveLayer>(document, layer);
        operation->execute();
    }
}

void Layers::replaceLayer(lc::Layer_CSPtr oldLayer, lc::Layer_CSPtr newLayer) {
    if(document != nullptr) {
        auto operation = std::make_shared<lc::operation::ReplaceLayer>(document, oldLayer, newLayer);
        operation->execute();
    }
}

void Layers::updateLayerList() {
    if(document != nullptr) {
        auto layersMap = document->allLayers();
        std::vector<lc::Layer_CSPtr> layersVector;

        for(auto layer : layersMap) {
            layersVector.push_back(layer.second);
        }

        model->setLayers(layersVector);
    }
}

void Layers::on_addLayerEvent(const lc::AddLayerEvent &) {
    updateLayerList();
}

void Layers::on_removeLayerEvent(const lc::RemoveLayerEvent &) {
    updateLayerList();
}

void Layers::on_replaceLayerEvent(const lc::ReplaceLayerEvent &) {
    updateLayerList();
}