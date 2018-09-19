#include "linepatternselect.h"

using namespace lc;
using namespace ui;

LinePatternSelect::LinePatternSelect(lc::Document_SPtr document, QWidget *parent, bool showByLayer, bool showByBlock) :
    QComboBox(parent),
    _showByLayer(showByLayer),
    _showByBlock(showByBlock) {

    qIconSize = QSize(128, 32);
    setIconSize(qIconSize);

    setMaximumHeight(32);

    setDocument(std::move(document));

    connect(this, SIGNAL(activated(const QString&)), this, SLOT(onActivated(const QString&)));
}

LinePatternSelect::LinePatternSelect(CadMdiChild* mdiChild, QWidget* parent, bool showByLayer, bool showByBlock) :
    LinePatternSelect((lc::Document_SPtr) nullptr, parent, showByLayer, showByBlock) {

    setMdiChild(mdiChild);
}

void LinePatternSelect::setMdiChild(CadMdiChild* mdiChild) {
    if(mdiChild == nullptr) {
        setDocument(nullptr);
    }
    else {
        setDocument(mdiChild->document());

        _metaInfoManager = mdiChild->metaInfoManager();
    }
}

void LinePatternSelect::setDocument(lc::Document_SPtr document) {
    if(_document != nullptr) {
        _document->addLinePatternEvent().disconnect<LinePatternSelect, &LinePatternSelect::on_addLinePatternEvent>(this);
        _document->removeLinePatternEvent().disconnect<LinePatternSelect, &LinePatternSelect::on_removeLinePatternEvent>(this);
        _document->replaceLinePatternEvent().disconnect<LinePatternSelect, &LinePatternSelect::on_replaceLinePatternEvent>(this);
    }

    _document = std::move(document);

    if(_document != nullptr) {
        _document->addLinePatternEvent().connect<LinePatternSelect, &LinePatternSelect::on_addLinePatternEvent>(this);
        _document->removeLinePatternEvent().connect<LinePatternSelect, &LinePatternSelect::on_removeLinePatternEvent>(this);
        _document->replaceLinePatternEvent().connect<LinePatternSelect, &LinePatternSelect::on_replaceLinePatternEvent>(this);
    }

    createEntries();
}

lc::DxfLinePattern_CSPtr LinePatternSelect::linePattern() {
    if(currentText() == BY_LAYER) {
        return nullptr;
    }

    if(currentText() == BY_BLOCK) {
        return std::make_shared<const lc::DxfLinePatternByBlock>();
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

QIcon LinePatternSelect::generateQIcon(lc::DxfLinePatternByValue_CSPtr linePattern) {
    QPixmap pixmap(qIconSize);

    LinePatternPainter painter(&pixmap, std::move(linePattern));
    painter.drawLinePattern();

    return QIcon(pixmap);
}

void LinePatternSelect::onActivated(const QString& text) {
    if(text.toStdString() == NEW_LP) {
        auto dialog = new AddLinePatternDialog(_document, this);
        dialog->show();

        if(_showByLayer) {
            setCurrentText(BY_LAYER);
        }
    }
    else if(text == MANAGE_LP) {
        auto dialog = new LinePatternManager(_document, this);
        dialog->show();

        if(_showByLayer) {
            setCurrentText(BY_LAYER);
        }
    }
    else if(_metaInfoManager != nullptr) {
        _metaInfoManager->setLinePattern(linePattern());
    }
}

void LinePatternSelect::createEntries() {
    clear();

    if(_document != nullptr) {
        addItem(NEW_LP);
        addItem(MANAGE_LP);
        insertSeparator(2);

        if(_showByLayer) {
            addItem(BY_LAYER);
            setCurrentText(BY_LAYER);
        }
        if(_showByBlock) {
            addItem(BY_BLOCK);
        }

        auto linePatterns = _document->linePatterns();
        for (const auto& linePattern : linePatterns) {
            auto lp = std::dynamic_pointer_cast<const lc::DxfLinePatternByValue>(linePattern);

            if(lp == nullptr) {
                continue;
            }

            auto icon = generateQIcon(lp);
            addItem(icon, linePattern->name().c_str());
        }
    }
}

void LinePatternSelect::on_addLinePatternEvent(const lc::AddLinePatternEvent& event) {
    createEntries();

    setCurrentText(event.linePattern()->name().c_str());
}

void LinePatternSelect::on_removeLinePatternEvent(const lc::RemoveLinePatternEvent& event) {
    createEntries();
}

void LinePatternSelect::on_replaceLinePatternEvent(const lc::ReplaceLinePatternEvent& event) {
    createEntries();

    setCurrentText(event.newLinePattern()->name().c_str());
}

void LinePatternSelect::onLayerChanged(const lc::Layer_CSPtr& layer) {
    auto index = findText(BY_LAYER);

    if(index != -1) {
        auto icon = generateQIcon(layer->linePattern());
        setItemIcon(index, icon);
    }
}