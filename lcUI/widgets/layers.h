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

                    /* --------- Lua GUI API Functions --------- */

                    /**
                     * \brief API function for adding a layer
                     */
                    void addLayer(lc::meta::Layer_CSPtr layer);
                    lc::meta::Layer_CSPtr addLayer(const char* layerName);
                    lc::meta::Layer_CSPtr addLayer(const char* layerName, double lineWidth);
                    lc::meta::Layer_CSPtr addLayer(const char* layerName, int r, int g, int b);
                    lc::meta::Layer_CSPtr addLayer(const char* layerName, lc::Color color);
                    lc::meta::Layer_CSPtr addLayer(const char* layerName, double lineWidth, int r, int g, int b);
                    lc::meta::Layer_CSPtr addLayer(const char* layerName, double lineWidth, lc::Color color);

                    /**
                     * \brief API function for removing a layer
                     * \param shared ptr to layer
                     */
                    void removeLayer(lc::meta::Layer_CSPtr layer);

                    /**
                    * \brief API function for removing a layer
                    * \param string layer name
                    */
                    void removeLayer(const char* layerName);

                    /**
                     * \brief API function for getting a layer
                     * \param string layer name
                     * \return shared ptr to layer
                     */
                    lc::meta::Layer_CSPtr layerByName(const char* layerName);

                    /**
                     * \brief API functions for renaming a layer
                     * \param shared ptr to layer
                     * \param string new layer name
                     */
                    void renameLayer(lc::meta::Layer_CSPtr layer, const char* newLayerName);

                    /**
                     * \brief API functions for renaming a layer
                     * \param string old layer name
                     * \param string new layer name
                     */
                    void renameLayer(const char* layerName, const char* newLayerName);

                    /**
                     * \brief API functions for replacing a layer
                     * \param shared ptr to old layer
                     * \param string shared ptr to newlayer
                     */
                    void replaceLayerAPI(lc::meta::Layer_CSPtr oldLayer, lc::meta::Layer_CSPtr newLayer);

                    /**
                     * \brief API functions for replacing a layer
                     * \param string old layer name
                     * \param string shared ptr to newlayer
                     */
                    void replaceLayerAPI(const char* oldLayerName, lc::meta::Layer_CSPtr newLayer);

                    /**
                     * \brief Get list of all layers
                     * \return vector of layers
                     */
                    std::vector<lc::meta::Layer_CSPtr> layers() const;

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
