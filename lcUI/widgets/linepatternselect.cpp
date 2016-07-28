#include "linepatternselect.h"

LinePatternSelect::LinePatternSelect(lc::Document_SPtr document, QWidget *parent, bool showByLayer, bool showByBlock) :
    QComboBox(parent),
    _showByLayer(showByLayer),
    _showByBlock(showByBlock) {

    qIconSize = QSize(256, 32);
    setIconSize(qIconSize);

    setDocument(document);

    connect(this, SIGNAL(activated(const QString&)), this, SLOT(onActivated(const QString&)));
}

void LinePatternSelect::setDocument(lc::Document_SPtr document) {
    if(_document != nullptr) {
        _document->addLinePatternEvent().disconnect<LinePatternSelect, &LinePatternSelect::on_addLinePatternEvent>(this);
        _document->removeLinePatternEvent().disconnect<LinePatternSelect, &LinePatternSelect::on_removeLinePatternEvent>(this);
        _document->replaceLinePatternEvent().disconnect<LinePatternSelect, &LinePatternSelect::on_replaceLinePatternEvent>(this);
    }

    _document = document;

    if(_document != nullptr) {
        _document->addLinePatternEvent().connect<LinePatternSelect, &LinePatternSelect::on_addLinePatternEvent>(this);
        _document->removeLinePatternEvent().connect<LinePatternSelect, &LinePatternSelect::on_removeLinePatternEvent>(this);
        _document->replaceLinePatternEvent().connect<LinePatternSelect, &LinePatternSelect::on_replaceLinePatternEvent>(this);
    }

    createEntries();
}

lc::DxfLinePattern_CSPtr LinePatternSelect::linePattern() {
    if(currentText() == BY_BLOCK || currentText() == BY_LAYER) {
        return nullptr;
    }

    auto linePatterns = _document->linePatterns();
    auto position = std::find_if(linePatterns.begin(), linePatterns.end(), [&](const lc::DxfLinePattern_CSPtr& item) {
        return item->name() == currentText().toStdString();
    });

    if(position != linePatterns.end()) {
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

void LinePatternSelect::onActivated(const QString& text) {
    if(text.toStdString() == NEW_LP) {
        auto dialog = new AddLinePatternDialog(_document, this);
        dialog->show();
    }
}

void LinePatternSelect::createEntries() {
    clear();

    if(_document != nullptr) {
        if(_showByBlock) {
            insertItem(0, BY_BLOCK);
        }

        if(_showByLayer) {
            insertItem(0, BY_LAYER);
        }

        insertItem(0, NEW_LP);

        auto linePatterns = _document->linePatterns();
        for (auto linePattern : linePatterns) {
            if (linePattern->name() == BY_BLOCK || linePattern->name() == BY_LAYER) {
                continue;
            }

            auto icon = generateQIcon(linePattern);
            addItem(icon, linePattern->name().c_str());
        }
    }
}

void LinePatternSelect::on_addLinePatternEvent(const lc::AddLinePatternEvent &) {
    createEntries();
}

void LinePatternSelect::on_removeLinePatternEvent(const lc::RemoveLinePatternEvent &) {
    createEntries();
}

void LinePatternSelect::on_replaceLinePatternEvent(const lc::ReplaceLinePatternEvent &) {
    createEntries();
}
