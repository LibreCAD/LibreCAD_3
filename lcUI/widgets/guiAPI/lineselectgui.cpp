#include "lineselectgui.h"

#include <QVBoxLayout>
#include "cad/interface/metatype.h"

using namespace lc::ui::api;

LineSelectGUI::LineSelectGUI(CadMdiChild* mdiChild, MetaInfoManager_SPtr metaInfoManager, std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    _linePatternSelect(mdiChild, this, true, true),
    _lineWidthSelect(metaInfoManager, this, true, true),
    _colorSelect(metaInfoManager, this, true, true)
{
    _linePatternSelect.setMetaInfoManager(metaInfoManager);
    QVBoxLayout* vboxlayout = new QVBoxLayout();
    vboxlayout->addWidget(&_linePatternSelect);
    vboxlayout->addWidget(&_lineWidthSelect);
    vboxlayout->addWidget(&_colorSelect);
    this->setLayout(vboxlayout);

    connect(&_linePatternSelect, &widgets::LinePatternSelect::linePatternChanged, this, &LineSelectGUI::metaInfoManagerChanged);
    connect(&_lineWidthSelect, &widgets::LineWidthSelect::lineWidthChanged, this, &LineSelectGUI::metaInfoManagerChanged);
    connect(&_colorSelect, &widgets::ColorSelect::colorChanged, this, &LineSelectGUI::metaInfoManagerChanged);
}

void LineSelectGUI::metaInfoManagerChanged() {
    // Phase 4 PR-5a — neutral callback invocation.
    for (auto& cb : _callbacks) {
        cb.invoke();
    }
}

void LineSelectGUI::addCallback(lc::scripting::ScriptCallback cb) {
    _callbacks.push_back(std::move(cb));
}

void LineSelectGUI::setEntityMetaInfo(lc::entity::CADEntity_CSPtr entity) {
    auto lineWidthInfo = entity->metaInfo<lc::meta::MetaLineWidthByValue>("_LINEWIDTH");
    auto lineColorInfo = entity->metaInfo<lc::meta::MetaColorByValue>("_COLOR");
    auto linePatternInfo = entity->metaInfo<lc::meta::DxfLinePatternByValue>("_LINEPATTERN");

    _lineWidthSelect.setWidth(lineWidthInfo);

    if (lineColorInfo != nullptr) {
        _colorSelect.setColor(lineColorInfo->color());
    }
    else {
        _colorSelect.setByLayer();
    }

    _linePatternSelect.setLinePattern(linePatternInfo);
}

void LineSelectGUI::getValue(lc::scripting::Map& map) {
}

void LineSelectGUI::hideLabel() {
}

void LineSelectGUI::copyValue(QDataStream& stream) {
}

void LineSelectGUI::pasteValue(QDataStream& stream) {
}
