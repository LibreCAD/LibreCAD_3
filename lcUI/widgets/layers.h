#pragma once

#include <cad/document/document.h>

#include <QDockWidget>
#include <QVBoxLayout>
#include <QMdiArea>
#include <QTableView>
#include <QMdiSubWindow>

#include "dialogs/addlayerdialog.h"
#include "layermodel.h"
#include <cad/operations/layerops.h>

namespace Ui {
    class Layers;
}

/**
 * \brief Widget which shows a list of layers
 */
class Layers : public QDockWidget {
    Q_OBJECT

    public:
        /**
         * \brief Create widget
         */
        Layers(CadMdiChild* mdiChild = nullptr, QWidget* parent = 0);
        ~Layers();

        /**
         * \brief Set new document.
         * \param document New document
         * Update the layer list.
         */
        void setMdiChild(CadMdiChild* mdiChild = nullptr);

    signals:
        void layerChanged(lc::Layer_CSPtr layer);

    private slots:
        void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
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
        CadMdiChild* _mdiChild;

        void updateLayerList();

        void on_addLayerEvent(const lc::AddLayerEvent&);
        void on_removeLayerEvent(const lc::RemoveLayerEvent&);
        void on_replaceLayerEvent(const lc::ReplaceLayerEvent&);
};