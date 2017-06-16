#include "colorselect.h"

using namespace lc;
using namespace ui;

ColorSelect::ColorSelect(lc::ui::MetaInfoManager_SPtr metaInfoManager, QWidget *parent, bool showByLayer, bool showByBlock) :
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

    addItem(CUSTOM);
    insertSeparator(count());

    for(auto color : QColor::colorNames()){
        QPixmap pixmap(qIconSize);
        pixmap.fill(color);
        addItem(QIcon(pixmap), color);
    }

    connect(this, SIGNAL(activated(const QString&)), this, SLOT(onActivated(const QString&)));

    setMetaInfoManager(metaInfoManager);
}

void ColorSelect::onActivated(const QString& text) {
    if(text == CUSTOM) {
        auto colorDialog = new QColorDialog(this);

        if(_customColor.isValid()) {
            colorDialog->setCurrentColor(_customColor);
        }
        colorDialog->show();

        connect(colorDialog, &QColorDialog::colorSelected, this, &ColorSelect::on_customColorChanged);
    }
    else {
        updateMetaInfoManager();
    }
}

void ColorSelect::on_customColorChanged(const QColor &color) {
    QPixmap pixmap(qIconSize);

    auto index = findText(CUSTOM);
    pixmap.fill(color);
    setItemIcon(index, QIcon(pixmap));

    _customColor = color;

    updateMetaInfoManager();
}

void ColorSelect::onLayerChanged(lc::Layer_CSPtr layer) {
    auto index = findText(BY_LAYER);

    if(index != -1) {
        QColor color(layer->color().redI(), layer->color().greenI(), layer->color().blueI(), layer->color().alphaI());
        QPixmap pixmap(qIconSize);
        pixmap.fill(color);
        setItemIcon(index, QIcon(pixmap));

        updateMetaInfoManager();
    }
}

void ColorSelect::setColor(lc::Color color) {
    QColor qColor(color.redI(), color.greenI(), color.blueI(), color.alphaI());
    setCurrentText(CUSTOM);

    on_customColorChanged(qColor);
    updateMetaInfoManager();
}

void ColorSelect::setMetaInfoManager(lc::ui::MetaInfoManager_SPtr metaInfoManager) {
    _metaInfoManager = metaInfoManager;

    if(metaInfoManager != nullptr && metaInfoManager->color() != nullptr) {
        setColor(metaInfoManager->color()->color());
    }
}

void ColorSelect::updateMetaInfoManager() {
    if(!_metaInfoManager) {
        return;
    }

    _metaInfoManager->setColor(metaColor());
}

lc::MetaColor_CSPtr ColorSelect::metaColor() {
    if(currentText() == BY_LAYER) {
        return nullptr;
    }
    if(currentText() == BY_BLOCK) {
        return nullptr;
    }

    return std::make_shared<const lc::MetaColor>(color());
}

lc::Color ColorSelect::color() {
    if(currentText() == BY_LAYER || currentText() == BY_BLOCK) {
        throw "Color can't be returned if ByLayer or ByBlock is selected.";
    }

    QColor color;

    if(currentText() == CUSTOM) {
        color = _customColor;
    }
    else {
        color = QColor(currentText());
    }

    if(!color.isValid()) {
        throw "The selected color is invalid.";
    }

    return lc::Color(color.red(), color.green(), color.blue(), color.alpha());
}