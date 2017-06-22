#pragma once

#include <QComboBox>
#include <QString>

#include <cad/meta/metalinewidth.h>
#include <widgets/linepatternpainter.h>
#include <managers/metainfomanager.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"

namespace lc {
    namespace ui {
        /**
         * \brief Dropdown select for line widths.
         * Line widths are hardcoded in the constructor
         */
        class LineWidthSelect : public QComboBox {
            Q_OBJECT

            public:
                /**
                 * \brief Create widget
                 * \param parent Pointer to parent widget
                 * \param showByLayer Add "ByLayer" option
                 * \param showByBlock Add "ByBlock" option
                 */
                LineWidthSelect(lc::ui::MetaInfoManager_SPtr metaInfoManager,
                                QWidget* parent = 0,
                                bool showByLayer = false, bool showByBlock = false);

                /**
                 * @brief Set the MetaInfo manager
                 * @param new MetaInfoManager or nullptr
                 */
                void setMetaInfoManager(lc::ui::MetaInfoManager_SPtr metaInfoManager);

                /**
                 * \brief Return selected line width
                 * \return Pointer to MetaLineWidth or nullptr if ByLayer or invalid value is selected
                 */
                lc::MetaLineWidth_CSPtr lineWidth();

                /**
                 * \brief Select a new width.
                 * \param width New LineWidth
                 * Does nothing if the width is not present in the values.
                 */
                void setWidth(lc::MetaLineWidth_CSPtr lineWidth);

            public slots:
                /**
                 * \brief Event when a new layer is selected.
                 * \param layer New selected layer
                 * This function update the "ByLayer" preview
                 */
                void onLayerChanged(lc::Layer_CSPtr layer);

            private slots:
                void onActivated(const QString& text);

            private:
                void createEntries();
                void updateMetaInfoManager();

                lc::ui::MetaInfoManager_SPtr _metaInfoManager;

                QSize qIconSize;

                std::map<QString, double> values;
        };
    }
}