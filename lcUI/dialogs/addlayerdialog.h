#pragma once

#include <QDialog>
#include <QColorDialog>
#include <QToolTip>
#include <QFormLayout>

#include "widgets/linepatternselect.h"

#include <cad/meta/layer.h>
#include <cad/meta/metalinewidth.h>

namespace Ui {
    class AddLayerDialog;
}

class AddLayerDialog : public QDialog {
    Q_OBJECT

    public:
        AddLayerDialog(std::vector<lc::DxfLinePattern_CSPtr> linePatterns, QWidget* parent = 0);
        ~AddLayerDialog();

    signals:
        void newLayer(lc::Layer_CSPtr);

    private slots:
        void accept();
        void on_pickColorButton_clicked();
        void on_colorChanged(const QColor& color);

    private:
        Ui::AddLayerDialog* ui;
        LinePatternSelect* linePatternSelect;
};