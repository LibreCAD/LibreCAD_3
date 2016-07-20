#pragma once

#include <cmath>

#include <QDialog>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>

#include <cad/meta/dxflinepattern.h>

#include "widgets/linepatternpathpart.h"
#include "widgets/linepatternpainter.h"

namespace Ui {
    class AddLinePatternDialog;
}

class AddLinePatternDialog : public QDialog {
    Q_OBJECT

    public:
        AddLinePatternDialog(QWidget* parent = 0);
        AddLinePatternDialog(lc::DxfLinePattern_CSPtr linePattern, QWidget* parent = 0);

        lc::DxfLinePattern_CSPtr linePattern();

    private slots:
        void onNewValuePressed();
        void generatePreview();

    private:
        Ui::AddLinePatternDialog* ui;
        QVBoxLayout* _layout;

        lc::DxfLinePattern_CSPtr _linePattern;
};