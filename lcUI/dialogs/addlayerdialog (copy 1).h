#pragma once
#include <cad/storage/document.h>

#include <QDialog>
#include <QColorDialog>
#include <QToolTip>
#include <QFormLayout>

#include "widgets/linepatternselect.h"
#include "widgets/linewidthselect.h"
#include "widgets/colorselect.h"

#include <cad/meta/layer.h>
#include <cad/meta/metalinewidth.h>

namespace Ui {
    class AddLayerDialog;
}

namespace lc {
    namespace ui {
        namespace dialog {
/**
 * \brief Create a dialog with all required fields to create a new layer
 */
            class AddLayerDialog : public QDialog {
                Q_OBJECT

                public:
                    /**
                     * \brief Create empty dialog
                     * \param document Document which contains the DXFLinePatterns
                     * \param parent Parent widget
                     */
                    AddLayerDialog(lc::storage::Document_SPtr document, QWidget* parent = 0);

                    /**
                     * \brief Create dialog pre-filled with existing Layer information
                     * \param oldLayer Layer to edit
                     * \param document Document which contains the DXFLinePatterns
                     * \param parent Parent widget
                     */
                    AddLayerDialog(lc::meta::Layer_CSPtr oldLayer, lc::storage::Document_SPtr document,
                                   QWidget* parent = 0);

                    ~AddLayerDialog();

                signals:
                    void newLayer(lc::meta::Layer_CSPtr);

                    void editLayer(lc::meta::Layer_CSPtr oldLayer, lc::meta::Layer_CSPtr newLayer);

                private slots:

                    /**
                     * \brief Create layer
                     * This function is called when "Ok" button is pressed.
                     * It creates the layer according to the entered information and send it back to Layers widget.
                     */
                    void accept();

                protected:
                    Ui::AddLayerDialog* ui;
                    lc::ui::widgets::LinePatternSelect* linePatternSelect;
                    lc::ui::widgets::LineWidthSelect* lineWidthSelect;
                    lc::ui::widgets::ColorSelect* colorSelect;

                private:
                    lc::meta::Layer_CSPtr _oldLayer;
            };
        }
    }
}