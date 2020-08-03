#include "lineselectgui.h"

#include <QVBoxLayout>

using namespace lc::ui::api;

LineSelectGUI::LineSelectGUI(CadMdiChild* mdiChild, std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    _linePatternSelect(mdiChild, this, true, true),
    _lineWidthSelect(mdiChild->metaInfoManager(), this, true, true),
    _colorSelect(mdiChild->metaInfoManager(), this, true, true)
{
    QVBoxLayout* vboxlayout = new QVBoxLayout();
    vboxlayout->addWidget(&_linePatternSelect);
    vboxlayout->addWidget(&_lineWidthSelect);
    vboxlayout->addWidget(&_colorSelect);
    this->setLayout(vboxlayout);
}

void LineSelectGUI::getLuaValue(kaguya::LuaRef& table) {

}

void LineSelectGUI::hideLabel() {

}

void LineSelectGUI::copyValue(QDataStream& stream) {

}

void LineSelectGUI::pasteValue(QDataStream& stream) {

}
