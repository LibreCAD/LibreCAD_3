#pragma once

#include <cad/document/document.h>

#include <QComboBox>

#include <cad/meta/dxflinepattern.h>
#include <widgets/linepatternpainter.h>
#include <dialogs/addlinepatterndialog.h>
#include <dialogs/linepatternmanager.h>
#include <cadmdichild.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"
#define NEW_LP "New line pattern"
#define MANAGE_LP "Manage line patterns"

namespace lc {
    namespace ui {
        /**
         * \brief Dropdown select for DxfLinePatterns
         */
        class LinePatternSelect : public QComboBox {
            Q_OBJECT

            public:
                /**
                 * \brief Create widget
                 * \param document Document containing the line patterns
                 * \param parent Pointer to parent widget
                 * \param showByLayer Add "ByLayer" option
                 * \param showByBlock Add "ByBlock" option
                 */
                LinePatternSelect(lc::Document_SPtr document = nullptr, QWidget* parent = 0, bool showByLayer = false,
                                  bool showByBlock = false);

                /**
                 * \brief Create widget (for drawing)
                 * \param document Document containing the line patterns
                 * \param parent Pointer to parent widget
                 * \param showByLayer Add "ByLayer" option
                 * \param showByBlock Add "ByBlock" option
                 */
                LinePatternSelect(CadMdiChild* mdiChild = nullptr, QWidget* parent = 0, bool showByLayer = false,
                                  bool showByBlock = false);

                /**
                 * \brief Change document
                 * \param document New document
                 */
                void setDocument(lc::Document_SPtr document = nullptr);

                /**
                 * @brief Set the window on which line pattern is applied
                 * @param mdiChild or nullptr
                 */
                void setMdiChild(CadMdiChild* mdiChild = nullptr);

                /**
                 * \brief Return selected line pattern
                 * \return Pointer to DxfLinePattern
                 */
                lc::DxfLinePattern_CSPtr linePattern();

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

                QIcon generateQIcon(lc::DxfLinePattern_CSPtr linePattern);

                QSize qIconSize;

                bool _showByLayer;
                bool _showByBlock;

                lc::Document_SPtr _document;
                lc::ui::MetaInfoManager_SPtr _metaInfoManager;

                void on_addLinePatternEvent(const lc::AddLinePatternEvent&);
                void on_removeLinePatternEvent(const lc::RemoveLinePatternEvent&);
                void on_replaceLinePatternEvent(const lc::ReplaceLinePatternEvent&);
        };
    }
}