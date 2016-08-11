#include <cad/meta/layer.h>
#include "linewidthselect.h"

LineWidthSelect::LineWidthSelect(QWidget* parent, bool showByLayer, bool showByBlock) :
    QComboBox(parent) {

    qIconSize = QSize(128, 32);
    setIconSize(qIconSize);

    setMaximumHeight(32);

    if(showByLayer) {
        addItem(BY_LAYER);
    }
    if(showByBlock) {
        addItem(BY_BLOCK);
    }

    values["0.00mm"] = 0.00;
    values["0.05mm"] = 0.05;
    values["0.09mm"] = 0.09;
    values["0.13mm (ISO)"] = 0.13;
    values["0.15mm"] = 0.15;
    values["0.18mm (ISO)"] = 0.18;
    values["0.20mm"] = 0.20;
    values["0.25mm (ISO)"] = 0.25;
    values["0.30mm"] = 0.30;
    values["0.35mm (ISO)"] = 0.35;
    values["0.40mm"] = 0.40;
    values["0.50mm (ISO)"] = 0.50;
    values["0.53mm"] = 0.53;
    values["0.60mm"] = 0.60;
    values["0.70mm (ISO)"] = 0.70;
    values["0.80mm"] = 0.80;
    values["0.90mm"] = 0.90;
    values["1.00mm (ISO)"] = 1.00;
    values["1.06mm"] = 1.06;
    values["1.20mm"] = 1.20;
    values["1.40mm (ISO)"] = 1.40;
    values["1.58mm"] = 1.58;
    values["2.00mm (ISO)"] = 2.00;
    values["2.11mm"] = 2.11;

    createEntries();
}

void LineWidthSelect::createEntries() {
    for(auto v : values) {
        QPixmap pixmap(qIconSize);

        LinePatternPainter painter(&pixmap, v.second);
        painter.drawLinePattern();

        addItem(QIcon(pixmap), v.first);
    }
}

lc::MetaLineWidthByValue_CSPtr LineWidthSelect::lineWidth() {
    if(currentText() == BY_BLOCK || currentText() == BY_LAYER) {
        return nullptr;
    }

    try {
        return std::make_shared<lc::MetaLineWidthByValue>(values.at(currentText()));
    }
    catch(std::out_of_range& e) {
        return std::make_shared<lc::MetaLineWidthByValue>(0);
    }
}

void LineWidthSelect::onLayerChanged(lc::Layer_CSPtr layer) {
    auto index = findText(BY_LAYER);

    if(index != -1) {
        QPixmap pixmap(qIconSize);

        LinePatternPainter painter(&pixmap, layer->lineWidth().width());
        painter.drawLinePattern();

        setItemIcon(index, QIcon(pixmap));
    }
}

void LineWidthSelect::setWidth(double width) {
    for(auto v : values) {
        if(v.second == width) {
            setCurrentText(v.first);
            break;
        }
    }
}