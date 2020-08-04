#include <cad/meta/layer.h>
#include "linewidthselect.h"

using namespace lc;
using namespace lc::ui::widgets;

LineWidthSelect::LineWidthSelect(lc::ui::MetaInfoManager_SPtr metaInfoManager, QWidget* parent, bool showByLayer, bool showByBlock) :
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

    setMetaInfoManager(std::move(metaInfoManager));

    connect(this, SIGNAL(activated(const QString&)), this, SLOT(onActivated(const QString&)));
}

void LineWidthSelect::createEntries() {
    for(auto v : values) {
        QPixmap pixmap(qIconSize);

        LinePatternPainter painter(&pixmap, v.second);
        painter.drawLinePattern();

        addItem(QIcon(pixmap), v.first);
    }
}

void LineWidthSelect::onLayerChanged(const lc::meta::Layer_CSPtr& layer) {
    auto index = findText(BY_LAYER);

    if(index != -1) {
        QPixmap pixmap(qIconSize);

        LinePatternPainter painter(&pixmap, layer->lineWidth().width());
        painter.drawLinePattern();

        setItemIcon(index, QIcon(pixmap));
        updateMetaInfoManager();
    }
}

void LineWidthSelect::setWidth(const lc::meta::MetaLineWidth_CSPtr& lineWidth) {
    if(lineWidth == nullptr) {
        setCurrentText(BY_LAYER);
        updateMetaInfoManager();
        return;
    }

    auto byValue = std::dynamic_pointer_cast<const lc::meta::MetaLineWidthByValue>(lineWidth);
    if(byValue != nullptr) {
        for (auto v : values) {
            if (v.second == byValue->width()) {
                setCurrentText(v.first);
                updateMetaInfoManager();
                break;
            }
        }

        return;
    }

    auto byBlock = std::dynamic_pointer_cast<const lc::meta::MetaLineWidthByBlock>(lineWidth);
    if(byBlock != nullptr) {
        setCurrentText(BY_BLOCK);
        updateMetaInfoManager();
        return;
    }
}

void LineWidthSelect::setMetaInfoManager(lc::ui::MetaInfoManager_SPtr metaInfoManager) {
    _metaInfoManager = std::move(metaInfoManager);

    if(_metaInfoManager != nullptr && _metaInfoManager->lineWidth() != nullptr) {
        setWidth(_metaInfoManager->lineWidth());
    }
}

void LineWidthSelect::updateMetaInfoManager() {
    if(!_metaInfoManager) {
        return;
    }

    if(currentText() == BY_LAYER) {
        _metaInfoManager->setLineWidth(nullptr);
    }else if(currentText() == BY_BLOCK) {
        _metaInfoManager->setLineWidth(std::make_shared<const lc::meta::MetaLineWidthByBlock>());
    }
    else {
        try {
            _metaInfoManager->setLineWidth(std::make_shared<lc::meta::MetaLineWidthByValue>(values.at(currentText())));
        }
        catch (std::out_of_range& e) {
            _metaInfoManager->setLineWidth(nullptr);
        }
    }

    emit lineWidthChanged();
}

lc::meta::MetaLineWidth_CSPtr LineWidthSelect::lineWidth() {
    if(currentText() == BY_LAYER) {
        return nullptr;
    }

    if(currentText() == BY_BLOCK) {
        return std::make_shared<const lc::meta::MetaLineWidthByBlock>();
    }

    try {
        return std::make_shared<lc::meta::MetaLineWidthByValue>(values.at(currentText()));
    }
    catch (std::out_of_range& e) {
        return nullptr;
    }
}

void LineWidthSelect::onActivated(const QString& text) {
    updateMetaInfoManager();
}
