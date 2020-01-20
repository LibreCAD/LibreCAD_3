#pragma once

#include <cad/storage/document.h>

#include <QDockWidget>
#include <QVBoxLayout>
#include <QMdiArea>
#include <QTableView>
#include <QMdiSubWindow>

#include "dialogs/addlayerdialog.h"
#include "layermodel.h"
#include <cad/operations/layerops.h>
#include "widgettitlebar.h"

namespace Ui {
    class Layers;
}

namespace lc {
    namespace ui {
        namespace widgets {
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

                    void layerChanged(lc::meta::Layer_CSPtr layer);

                private slots:

                    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

                    void on_newButton_clicked();

                    void on_deleteButton_clicked();

                    void on_layerList_clicked(const QModelIndex& index);

                    void changeLayerName(lc::meta::Layer_CSPtr& layer, const std::string& name);

                protected:
                    Ui::Layers* ui;
                    LayerModel* model;

                    void createLayer(lc::meta::Layer_CSPtr layer);

                    void deleteLayer(lc::meta::Layer_CSPtr layer);

                    void replaceLayer(lc::meta::Layer_CSPtr oldLayer, lc::meta::Layer_CSPtr newLayer);

                private:
                    CadMdiChild* _mdiChild;

                    void updateLayerList();

                    void on_addLayerEvent(const lc::event::AddLayerEvent&);

                    void on_removeLayerEvent(const lc::event::RemoveLayerEvent&);

                    void on_replaceLayerEvent(const lc::event::ReplaceLayerEvent&);

					void closeEvent(QCloseEvent* event);
            };
        }
    }
}