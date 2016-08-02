#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QMdiArea>
#include <QTableView>
#include <QMdiSubWindow>

#include "dialogs/addlayerdialog.h"
#include "layermodel.h"
#include <cad/document/document.h>
#include <cad/operations/layerops.h>

namespace Ui {
    class Layers;
}

class Layers : public QDockWidget {
    Q_OBJECT

    public:
        Layers(lc::Document_SPtr document = nullptr, QWidget* parent = 0);
        ~Layers();

        void setDocument(lc::Document_SPtr document = nullptr);
        lc::Layer_CSPtr activeLayer();

    private slots:
        void on_newButton_clicked();
        void on_deleteButton_clicked();
        void on_layerList_clicked(const QModelIndex& index);

        void changeLayerName(lc::Layer_CSPtr& layer, const std::string& name);

    protected:
        Ui::Layers* ui;
        LayerModel* model;
        void createLayer(lc::Layer_CSPtr layer);
        void deleteLayer(lc::Layer_CSPtr layer);
        void replaceLayer(lc::Layer_CSPtr oldLayer, lc::Layer_CSPtr newLayer);

    private:
        lc::Document_SPtr _document;

        void updateLayerList();

        void on_addLayerEvent(const lc::AddLayerEvent&);
        void on_removeLayerEvent(const lc::RemoveLayerEvent&);
        void on_replaceLayerEvent(const lc::ReplaceLayerEvent&);
};