#include "lineselectgui.h"

#include <QVBoxLayout>

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
    for (int i = 0; i < _callbacks.size(); i++) {
        _callbacks[i]();
    }
}

void LineSelectGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void LineSelectGUI::getLuaValue(kaguya::LuaRef& table) {

}

void LineSelectGUI::hideLabel() {

}

void LineSelectGUI::copyValue(QDataStream& stream) {

}

void LineSelectGUI::pasteValue(QDataStream& stream) {

}
