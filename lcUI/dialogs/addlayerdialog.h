#pragma once
#include <cad/document/document.h>

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
        AddLayerDialog(lc::Document_SPtr document, QWidget* parent = 0);

        /**
         * \brief Create dialog pre-filled with existing Layer information
         * \param oldLayer Layer to edit
         * \param document Document which contains the DXFLinePatterns
         * \param parent Parent widget
         */
        AddLayerDialog(lc::Layer_CSPtr oldLayer, lc::Document_SPtr document, QWidget* parent = 0);
        ~AddLayerDialog();

    signals:
        void newLayer(lc::Layer_CSPtr);
        void editLayer(lc::Layer_CSPtr oldLayer, lc::Layer_CSPtr newLayer);

    private slots:
        /**
         * \brief Create layer
         * This function is called when "Ok" button is pressed.
         * It creates the layer according to the entered information and send it back to Layers widget.
         */
        void accept();

    protected:
        Ui::AddLayerDialog* ui;
        lc::ui::LinePatternSelect* linePatternSelect;
        lc::ui::LineWidthSelect* lineWidthSelect;
        lc::ui::ColorSelect* colorSelect;

    private:
        lc::Layer_CSPtr _oldLayer;
};