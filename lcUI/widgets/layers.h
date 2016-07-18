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
        Layers(QMdiArea* mdiArea, QWidget* parent = 0);
        ~Layers();

        lc::Layer_CSPtr activeLayer();

    private slots:
        void on_newButton_clicked();
        void on_deleteButton_clicked();
        void on_layerList_clicked(const QModelIndex& index);
        void onSelectionChanged(const QItemSelection& selected, const QItemSelection&);
        void onSubwindowActivated(QMdiSubWindow *window);

        void createLayer(lc::Layer_CSPtr layer);
        void deleteLayer(lc::Layer_CSPtr layer);
        void replaceLayer(lc::Layer_CSPtr oldLayer, lc::Layer_CSPtr newLayer);

        void changeLayerName(lc::Layer_CSPtr& layer, const std::string& name);

    private:
        Ui::Layers* ui;
        LayerModel* model;
        std::shared_ptr<lc::Document> document;

        void updateLayerList();

        void on_addLayerEvent(const lc::AddLayerEvent&);
        void on_removeLayerEvent(const lc::RemoveLayerEvent&);
        void on_replaceLayerEvent(const lc::ReplaceLayerEvent&);
};