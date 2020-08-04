#pragma once

#include <QComboBox>
#include <QString>
#include <QColor>
#include <QColorDialog>

#include <cad/meta/metacolor.h>
#include <cad/meta/layer.h>
#include <managers/metainfomanager.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"
#define CUSTOM "Custom"

namespace lc {
    namespace ui {
        namespace widgets {
            /**
             * Dropdown select for colors.
             * Contains all colors from QColor::colorNames().
             */
            class ColorSelect : public QComboBox {
                Q_OBJECT

                public:
                    /**
                     * \brief Create widget.
                     * \param parent Pointer to parent widget
                     * \param showByLayer Add "ByLayer" entry
                     * \param showByBlock Add "ByBlock" entry
                     */
                    ColorSelect(lc::ui::MetaInfoManager_SPtr metaInfoManager = nullptr,
                                QWidget* parent = 0,
                                bool showByLayer = false, bool showByBlock = false);

                    /**
                     * @brief Set the MetaInfo manager
                     * @param new MetaInfoManager or nullptr
                     */
                    void setMetaInfoManager(lc::ui::MetaInfoManager_SPtr metaInfoManager = nullptr);

                    /**
                     * \brief Returns selected color
                     * \return Shared pointer to MetaColor
                     */
                    lc::meta::MetaColor_CSPtr metaColor();

                    /**
                     * \brief Returns selected color
                     * \return Color
                     */
                    lc::Color color();

                    /**
                     * \brief Set selected color.
                     * \param color New selected color
                     * This set the current selected item to Custom with the new color
                     */
                    void setColor(const lc::Color& color);

                signals:
                    void colorChanged();

                public slots:

                    /**
                     * \brief Event when a new layer is selected.
                     * \param layer New selected layer
                     * This updates "ByLayer" color preview.
                     */
                    void onLayerChanged(const lc::meta::Layer_CSPtr& layer);

                private slots:

                    void onActivated(const QString& text);

                    void on_customColorChanged(const QColor& color);

                private:
                    void updateMetaInfoManager();

                    QSize qIconSize;
                    QColor _customColor;
                    lc::ui::MetaInfoManager_SPtr _metaInfoManager;
            };
        }
    }
}
