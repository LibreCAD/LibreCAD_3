#pragma once
#include <cad/document/document.h>
#include <cmath>

#include <QDialog>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>

#include <cad/meta/dxflinepattern.h>
#include <cad/operations/linepatternops.h>


#include "widgets/linepatternpathpart.h"
#include "widgets/linepatternpainter.h"

#define SAVE_AS_NEW_TEXT "Save as new"
#define EDIT_TEXT "Edit"

namespace Ui {
    class AddLinePatternDialog;
}
/**
 * \brief Create a dialog with all required fields to create a new line pattern
 */
class AddLinePatternDialog : public QDialog {
    Q_OBJECT

    public:
        /**
         * \brief Create empty dialog
         * \param document Document which contains the DXFLinePatterns
         * \param parent Parent widget
         */
        AddLinePatternDialog(lc::Document_SPtr document, QWidget* parent = 0);

        /**
         * \brief Create dialog pre-filled with existing line pattern information.
         * \param document Document which contains the DXFLinePatterns.
         * \param linePattern Line pattern to edit
         * \param parent Parent widget
         */
        AddLinePatternDialog(lc::Document_SPtr document, lc::DxfLinePatternByValue_CSPtr linePattern, QWidget* parent = 0);

    private slots:
        /**
         * \brief Create a new entry for the pattern path.
         */
        void on_newValueButton_pressed();

        /**
         * \brief Cancel
         * Close the dialog.
         */
        void on_cancelButton_pressed();

        /**
         * \brief Save
         * Save the line pattern in the document and close the dialog.
         */
        void on_saveButton_pressed();

        /**
         * \brief Edit
         * Replace the original line pattern with the new one and close the dialog.
         */
        void onEditButtonPressed();

        /**
         * \brief Generate line pattern preview
         * Create a new QPixmap with a preview of the line pattern and add it in the dialog.
         */
        void generatePreview();

    protected:
        Ui::AddLinePatternDialog* ui;
        QPushButton* editButton;
        QVBoxLayout* _layout;

    private:
        lc::Document_SPtr _document;

        lc::DxfLinePatternByValue_CSPtr _linePattern;
        lc::DxfLinePatternByValue_CSPtr _oldLinePattern;
};