#include "colorselect.h"

ColorSelect::ColorSelect(QWidget *parent, bool showByLayer, bool showByBlock) :
    QComboBox(parent) {

    qIconSize = QSize(32, 32);
    setIconSize(qIconSize);

    setMaximumHeight(32);

    if(showByLayer) {
        addItem(BY_LAYER);
    }
    if(showByBlock) {
        addItem(BY_BLOCK);
    }

    for(auto color : QColor::colorNames()){
        QPixmap pixmap(qIconSize);
        pixmap.fill(color);
        addItem(QIcon(pixmap), color);
    }
}

lc::MetaColor_CSPtr ColorSelect::color() {
    QColor color(currentText());

    if(!color.isValid()) {
        return nullptr;
    }

    return std::make_shared<lc::MetaColor>(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}