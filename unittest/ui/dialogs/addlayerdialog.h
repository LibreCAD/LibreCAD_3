#pragma once

#include <dialogs/addlayerdialog.cpp>

class AddLayerDialogTest : public AddLayerDialog {
    public:
        AddLayerDialogTest(lc::Document_SPtr document, QWidget* parent = 0) :
                AddLayerDialog(document, parent) {
        }

        AddLayerDialogTest(lc::Layer_CSPtr oldLayer, lc::Document_SPtr document, QWidget* parent = 0) :
                AddLayerDialog(oldLayer, document, parent) {
        }

        std::string layerName() {
            return ui->name->text().toStdString();
        }

        std::string linePatternName() {
            if(linePatternSelect->linePattern() == nullptr) {
                return "";
            }
            return linePatternSelect->linePattern()->name();
        };

        double lineWidthValue() {
            return std::dynamic_pointer_cast<const lc::MetaLineWidthByValue>(lineWidthSelect->lineWidth())->width();
        };

        lc::Color color() {
            return colorSelect->color();
        };
};