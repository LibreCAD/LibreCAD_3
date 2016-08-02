#include <cadmdichild.h>
#include "layers.h"
#include "ui_layers.h"

Layers::Layers(lc::Document_SPtr document, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Layers) {

    ui->setupUi(this);

    model = new LayerModel(this);
    setDocument(document);

    ui->layerList->setModel(model);
    ui->layerList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->layerList->horizontalHeader()->setSectionResizeMode(LayerModel::NAME, QHeaderView::Stretch);

    connect(model, &LayerModel::nameChanged, this, &Layers::changeLayerName);
}

Layers::~Layers() {
    delete ui;
}

void Layers::setDocument(lc::Document_SPtr document) {
    if(_document != nullptr) {
        _document->addLayerEvent().disconnect<Layers, &Layers::on_addLayerEvent>(this);
        _document->removeLayerEvent().disconnect<Layers, &Layers::on_removeLayerEvent>(this);
        _document->replaceLayerEvent().disconnect<Layers, &Layers::on_replaceLayerEvent>(this);
    }

    _document = document;

    if(_document != nullptr) {
        _document->addLayerEvent().connect<Layers, &Layers::on_addLayerEvent>(this);
        _document->removeLayerEvent().connect<Layers, &Layers::on_removeLayerEvent>(this);
        _document->replaceLayerEvent().connect<Layers, &Layers::on_replaceLayerEvent>(this);
    }

    updateLayerList();
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
    auto dialog = new AddLayerDialog(_document, this);
    dialog->show();

    connect(dialog, &AddLayerDialog::newLayer, this, &Layers::createLayer);
}

void Layers::on_deleteButton_clicked() {
    QModelIndexList selection = ui->layerList->selectionModel()->selectedRows();

    deleteLayer(model->layerAt(selection.first().row()));
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

            case LayerModel::EDIT:
                auto dialog = new AddLayerDialog(layer, _document, this);
                dialog->show();

                connect(dialog, &AddLayerDialog::editLayer, this, &Layers::replaceLayer);
                return;
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
    if(_document != nullptr) {
        auto operation = std::make_shared<lc::operation::AddLayer>(_document, layer);
        operation->execute();
    }
}

void Layers::deleteLayer(lc::Layer_CSPtr layer) {
    if(_document != nullptr) {
        auto operation = std::make_shared<lc::operation::RemoveLayer>(_document, layer);
        operation->execute();
    }
}

void Layers::replaceLayer(lc::Layer_CSPtr oldLayer, lc::Layer_CSPtr newLayer) {
    if(_document != nullptr) {
        auto operation = std::make_shared<lc::operation::ReplaceLayer>(_document, oldLayer, newLayer);
        operation->execute();
    }
}

void Layers::updateLayerList() {
    std::vector<lc::Layer_CSPtr> layersVector;

    if(_document != nullptr) {
        auto layersMap = _document->allLayers();

        for(auto layer : layersMap) {
            layersVector.push_back(layer.second);
        }
    }

    model->setLayers(layersVector);
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