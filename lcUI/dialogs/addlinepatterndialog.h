#pragma once

#include <cmath>

#include <QDialog>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>

#include <cad/meta/dxflinepattern.h>
#include <cad/operations/linepatternops.h>
#include <cad/document/document.h>

#include "widgets/linepatternpathpart.h"
#include "widgets/linepatternpainter.h"

#define SAVE_AS_NEW_TEXT "Save as new"
#define EDIT_TEXT "Edit"

namespace Ui {
    class AddLinePatternDialog;
}

class AddLinePatternDialog : public QDialog {
    Q_OBJECT

    public:
        AddLinePatternDialog(lc::Document_SPtr document, QWidget* parent = 0);
        AddLinePatternDialog(lc::Document_SPtr document, lc::DxfLinePattern_CSPtr linePattern, QWidget* parent = 0);

    private slots:
        void on_newValueButton_pressed();
        void on_cancelButton_pressed();
        void on_saveButton_pressed();
        void onEditButtonPressed();

        void generatePreview();

    protected:
        Ui::AddLinePatternDialog* ui;
        QPushButton* editButton;
        QVBoxLayout* _layout;

    private:
        lc::Document_SPtr _document;

        lc::DxfLinePattern_CSPtr _linePattern;
        lc::DxfLinePattern_CSPtr _oldLinePattern;
};