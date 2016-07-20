#include "linepatternselect.h"

LinePatternSelect::LinePatternSelect(std::vector<lc::DxfLinePattern_CSPtr>& linePatterns, QWidget *parent) :
    QComboBox(parent),
    _linePatterns(linePatterns) {

    qIconSize = QSize(256, 32);
    setIconSize(qIconSize);

    for(auto linePattern : linePatterns) {
        if(linePattern->name() == BY_BLOCK || linePattern->name() == BY_LAYER) {
            continue;
        }

        auto icon = generateQIcon(linePattern);
        addItem(icon, linePattern->name().c_str());
    }
}

void LinePatternSelect::addByBlock() {
    insertItem(0, BY_BLOCK);
}

void LinePatternSelect::addByLayer() {
    insertItem(0, BY_LAYER);
}

lc::DxfLinePattern_CSPtr LinePatternSelect::linePattern() {
    if(currentText() == BY_BLOCK || currentText() == BY_LAYER) {
        return nullptr;
    }

    auto position = std::find_if(_linePatterns.begin(), _linePatterns.end(), [&](const lc::DxfLinePattern_CSPtr& item) {
        return item->name() == currentText().toStdString();
    });

    if(position != _linePatterns.end()) {
        return *position;
    }
    else {
        return nullptr;
    }
}

QIcon LinePatternSelect::generateQIcon(lc::DxfLinePattern_CSPtr linePattern) {
    QPixmap pixmap(qIconSize);

    LinePatternPainter painter(linePattern, &pixmap);
    painter.drawLinePattern();

    return QIcon(pixmap);
}