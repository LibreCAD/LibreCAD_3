#pragma once

#include <QAbstractTableModel>
#include <QIcon>

#include <cad/meta/layer.h>

namespace lc {
    namespace ui {
        namespace widgets {
            /**
             * \brief Model to display a list of layers in a QTableView
             */
            class LayerModel : public QAbstractTableModel {
                Q_OBJECT

                public:
                    enum columns {
                        NAME,
                        EDIT,
                        LOCKED,
                        LAST
                    };

                    /**
                     * \brief Create widget
                     * \param parent Pointer to parent object
                     */
                    LayerModel(QObject* parent = nullptr);

                    /**
                     * \brief Update the list of layers.
                     * \param layers Vector containing new layers
                     */
                    void setLayers(std::vector<lc::meta::Layer_CSPtr> layers);

                    /**
                     * \brief Get layer at specific row.
                     * \param row Row in the list
                     * \return Shared pointer to Layer
                     */
                    lc::meta::Layer_CSPtr layerAt(int row);

                    /**
                     * \brief Get layer with given name.
                     * \param layerName name of the layer
                     * \return Shared pointer to Layer
                     */
                    lc::meta::Layer_CSPtr layerByName(const std::string& layerName);

                    /**
                     * @brief Give the index of the given layer
                     * @param layer
                     * @return Index of layer
                     */
                    unsigned int indexOf(const lc::meta::Layer_CSPtr& layer);

                    /**
                     * \brief Get number of layers
                     * \return Number of layers
                     */
                    int rowCount(const QModelIndex& parent) const override;

                    int columnCount(const QModelIndex& parent) const override;

                    QVariant data(const QModelIndex& index, int role) const override;

                    Qt::ItemFlags flags(const QModelIndex& index) const override;

                    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

                    std::vector<lc::meta::Layer_CSPtr> layers() const;

                signals:

                    /**
                     * \brief Signal when layer name was changed by double clicking
                     */
                    void nameChanged(lc::meta::Layer_CSPtr& layer, const std::string& name);

                private:
                    std::vector<lc::meta::Layer_CSPtr> _layers;

                    QIcon _editIcon;
                    QIcon _lockedIcon;
                    QIcon _unlockedIcon;
            };
        }
    }
}
