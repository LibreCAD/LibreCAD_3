#include <cadmdichild.h>
#include <QtWidgets/QMessageBox>
#include "layers.h"
#include "ui_layers.h"

using namespace lc;
using namespace lc::ui;
using namespace lc::ui::widgets;

Layers::Layers(CadMdiChild* mdiChild, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Layers),
    _mdiChild(nullptr) {

    ui->setupUi(this);

    model = new LayerModel(this);
    setMdiChild(mdiChild);

    ui->layerList->setModel(model);
    ui->layerList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->layerList->horizontalHeader()->setSectionResizeMode(LayerModel::NAME, QHeaderView::Stretch);

    connect(model, &LayerModel::nameChanged, this, &Layers::changeLayerName);
    connect(ui->layerList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

	WidgetTitleBar* titleBar = new WidgetTitleBar( "Layers", this,
													WidgetTitleBar::TitleBarOptions::VerticalOnHidden);
	this->setTitleBarWidget(titleBar);
}

Layers::~Layers() {
    delete ui;
}

void Layers::setMdiChild(CadMdiChild* mdiChild) {
    if(_mdiChild != nullptr) {
        auto document = _mdiChild->document();
        document->addLayerEvent().disconnect<Layers, &Layers::on_addLayerEvent>(this);
        document->removeLayerEvent().disconnect<Layers, &Layers::on_removeLayerEvent>(this);
        document->replaceLayerEvent().disconnect<Layers, &Layers::on_replaceLayerEvent>(this);
    }

    _mdiChild = mdiChild;

    if(mdiChild != nullptr) {
        auto document = mdiChild->document();
        document->addLayerEvent().connect<Layers, &Layers::on_addLayerEvent>(this);
        document->removeLayerEvent().connect<Layers, &Layers::on_removeLayerEvent>(this);
        document->replaceLayerEvent().connect<Layers, &Layers::on_replaceLayerEvent>(this);
    }

    updateLayerList();
}

void Layers::on_newButton_clicked() {
    if(_mdiChild == nullptr) {
        return;
    }

    auto dialog = new dialog::AddLayerDialog(_mdiChild->document(), this);
    dialog->show();

    connect(dialog, &dialog::AddLayerDialog::newLayer, this, &Layers::createLayer);
}

void Layers::on_deleteButton_clicked() {
    QModelIndexList selection = ui->layerList->selectionModel()->selectedRows();

    deleteLayer(model->layerAt(selection.first().row()));
}

void Layers::on_layerList_clicked(const QModelIndex& index) {
    if(!index.isValid() || _mdiChild == nullptr) {
        return;
    }

    auto layer = model->layerAt(index.row());
    if(index.column() != LayerModel::NAME) {
        auto locked = layer->isFrozen();

        switch (index.column()) {
            case LayerModel::LOCKED:
                locked = !locked;
                break;

            case LayerModel::EDIT:
                auto dialog = new dialog::AddLayerDialog(layer, _mdiChild->document(), this);
                dialog->show();

                connect(dialog, &dialog::AddLayerDialog::editLayer, this, &Layers::replaceLayer);
                return;
        }

        auto newLayer = std::make_shared<const lc::meta::Layer>(
                layer->name(),
                layer->lineWidth(),
                layer->color(),
                layer->linePattern(),
                locked
        );

        replaceLayer(layer, newLayer);
    }
}

void Layers::changeLayerName(lc::meta::Layer_CSPtr& layer, const std::string& name) {
    if(name.empty()) {
        return;
    }

    auto newLayer = std::make_shared<const lc::meta::Layer>(
        name,
        layer->lineWidth(),
        layer->color(),
        layer->linePattern(),
        layer->isFrozen()
    );

    replaceLayer(layer, newLayer);
}

void Layers::createLayer(lc::meta::Layer_CSPtr layer) {
    if(_mdiChild != nullptr) {
        auto operation = std::make_shared<lc::operation::AddLayer>(_mdiChild->document(), layer);
        operation->execute();
    }
}

void Layers::deleteLayer(lc::meta::Layer_CSPtr layer) {
    if(_mdiChild != nullptr) {
        try {
            auto operation = std::make_shared<lc::operation::RemoveLayer>(_mdiChild->document(), layer);
            operation->execute();
        }
        catch(const char* message) {
            QMessageBox::critical(nullptr, "Error", message);
        }
    }
}

void Layers::replaceLayer(lc::meta::Layer_CSPtr oldLayer, lc::meta::Layer_CSPtr newLayer) {
    if(_mdiChild != nullptr) {
        auto operation = std::make_shared<lc::operation::ReplaceLayer>(_mdiChild->document(), oldLayer, newLayer);
        operation->execute();
    }
}

void Layers::updateLayerList() {
    std::vector<lc::meta::Layer_CSPtr> layersVector;

    if (_mdiChild != nullptr) {
        auto layersMap = _mdiChild->document()->allLayers();

        for (auto layer : layersMap) {
            layersVector.insert(layersVector.begin(), layer.second);
        }
    }

    model->setLayers(layersVector);

    if(_mdiChild != nullptr && _mdiChild->activeLayer() != nullptr) {
        auto layer = _mdiChild->activeLayer();
        ui->layerList->selectRow(model->indexOf(layer));

        emit layerChanged(layer);
    }

    QPushButton* deleteBut = this->findChild<QPushButton*>(QString("deleteButton"));
    //check if only one layer is remaining, in which case disable button
    if(model->rowCount(QModelIndex()) == 1)
    {
        deleteBut->setEnabled(false);
    }
    else
    {
        deleteBut->setEnabled(true);
    }
}

void Layers::on_addLayerEvent(const lc::event::AddLayerEvent& event) {
    _mdiChild->setActiveLayer(event.layer());

    updateLayerList();
}

void Layers::on_removeLayerEvent(const lc::event::RemoveLayerEvent& event) {
    if(_mdiChild->activeLayer() == event.layer()) {
        _mdiChild->setActiveLayer(model->layerAt(0));
    }

    updateLayerList();
}

void Layers::on_replaceLayerEvent(const lc::event::ReplaceLayerEvent& event) {
    if(_mdiChild->activeLayer() == event.oldLayer()) {
        _mdiChild->setActiveLayer(event.newLayer());
    }

    updateLayerList();
}

void Layers::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    if(selected.isEmpty()) {
        return;
    }

    auto layer = model->layerAt(selected.first().top());
    _mdiChild->setActiveLayer(layer);
    emit layerChanged(layer);
}

void Layers::closeEvent(QCloseEvent* event)
{
	this->widget()->hide();
	event->ignore();
}

/* Lua GUI API Functions */
void Layers::addLayer(lc::meta::Layer_CSPtr layer) {
    if (layer != nullptr) {
        createLayer(layer);
    }
}

lc::meta::Layer_CSPtr Layers::addLayer(const char* layerName) {
    lc::meta::Layer_CSPtr layer = std::make_shared<const lc::meta::Layer>(
        std::string(layerName),
        1.00,
        lc::Color(255, 255, 255)
        );
    addLayer(layer);
    return layer;
}

lc::meta::Layer_CSPtr Layers::addLayer(const char* layerName, double lineWidth) {
    lc::meta::Layer_CSPtr layer = std::make_shared<const lc::meta::Layer>(
        std::string(layerName),
        lineWidth,
        lc::Color(255, 255, 255)
        );
    addLayer(layer);
    return layer;
}

lc::meta::Layer_CSPtr Layers::addLayer(const char* layerName, int r, int g, int b) {
    lc::meta::Layer_CSPtr layer = std::make_shared<const lc::meta::Layer>(
        std::string(layerName),
        1.00,
        lc::Color(r, g, b)
        );
    addLayer(layer);
    return layer;
}

lc::meta::Layer_CSPtr Layers::addLayer(const char* layerName, double lineWidth, int r, int g, int b) {
    lc::meta::Layer_CSPtr layer = std::make_shared<const lc::meta::Layer>(
        std::string(layerName),
        lineWidth,
        lc::Color(r,g,b)
        );
    addLayer(layer);
    return layer;
}

lc::meta::Layer_CSPtr Layers::addLayer(const char* layerName, lc::Color color) {
    lc::meta::Layer_CSPtr layer = std::make_shared<const lc::meta::Layer>(
        std::string(layerName),
        1.00,
        color
        );
    addLayer(layer);
    return layer;
}

lc::meta::Layer_CSPtr Layers::addLayer(const char* layerName, double lineWidth, lc::Color color) {
    lc::meta::Layer_CSPtr layer = std::make_shared<const lc::meta::Layer>(
        std::string(layerName),
        lineWidth,
        color
        );
    addLayer(layer);
    return layer;
}

void Layers::removeLayer(lc::meta::Layer_CSPtr layer) {
    if (layer != nullptr) {
        deleteLayer(layer);
    }
}

void Layers::removeLayer(const char* layerName) {
    lc::meta::Layer_CSPtr layer = model->layerByName(layerName);
    removeLayer(layer);
}

lc::meta::Layer_CSPtr Layers::layerByName(const char* layerName) {
    return model->layerByName(layerName);
}

void Layers::renameLayer(lc::meta::Layer_CSPtr layer, const char* newLayerName) {
    if (layer != nullptr) {
        changeLayerName(layer, newLayerName);
    }
}

void Layers::renameLayer(const char* layerName, const char* newLayerName) {
    lc::meta::Layer_CSPtr layer = model->layerByName(layerName);
    renameLayer(layer, newLayerName);
}

void Layers::replaceLayerAPI(lc::meta::Layer_CSPtr oldLayer, lc::meta::Layer_CSPtr newLayer) {
    if (oldLayer != nullptr && newLayer != nullptr) {
        replaceLayer(oldLayer, newLayer);
    }
}

void Layers::replaceLayerAPI(const char* oldLayerName, lc::meta::Layer_CSPtr newLayer) {
    lc::meta::Layer_CSPtr oldLayer = model->layerByName(oldLayerName);
    replaceLayer(oldLayer, newLayer);
}

std::vector<lc::meta::Layer_CSPtr> Layers::layers() const {
    return model->layers();
}
