#include "lwvertexgroup.h"

using namespace lc::ui::api;

LWVertexGroup::LWVertexGroup(std::string label, QWidget* parent)
    :
    HorizontalGroupGUI(label, true, parent)
{
}

void LWVertexGroup::addCallback(lc::scripting::ScriptCallback cb) {
    coordgui->addFinishCallback(cb);
    startWidth->addCallback(cb);
    endWidth->addCallback(cb);
}

void LWVertexGroup::setMainWindow(lc::ui::MainWindow* mainWindowIn) {
    coordgui->enableCoordinateSelection(mainWindowIn);
}

void LWVertexGroup::getValue(lc::scripting::Map& map) {
    HorizontalGroupGUI::getValue(map);

    // Phase 4 PR-5b — nested-Map aliasing.  Preserve the LuaInterface
    // semantics where the nested table under _key holds the same
    // per-child ScriptValues that live at the top level.
    auto nested = lc::scripting::makeMap();
    (*nested)[_key + "_Location"]   = (*map)[_key + "_Location"];
    (*nested)[_key + "_StartWidth"] = (*map)[_key + "_StartWidth"];
    (*nested)[_key + "_EndWidth"]   = (*map)[_key + "_EndWidth"];
    (*nested)[_key + "_Bulge"]      = lc::scripting::ScriptValue(_bulge);
    (*map)[_key] = lc::scripting::ScriptValue(nested);
}

void LWVertexGroup::setKey(const std::string& keyIn) {
    InputGUI::setKey(keyIn);

    coordgui = new lc::ui::api::CoordinateGUI("Location");
    startWidth = new lc::ui::api::NumberGUI("Start Width");
    endWidth = new lc::ui::api::NumberGUI("End Width");

    coordgui->hideLabel();
    startWidth->layout()->removeItem(startWidth->layout()->itemAt(1));
    endWidth->layout()->removeItem(endWidth->layout()->itemAt(1));
    qboxlayout->setContentsMargins(QMargins(0, 0, 0, 0));

    addWidget(_key + "_Location", coordgui);
    addWidget(_key + "_StartWidth", startWidth);
    addWidget(_key + "_EndWidth", endWidth);
}

void LWVertexGroup::setValue(lc::geo::Coordinate coord, double sw, double ew, double bulge) {
    coordgui->setValue(coord);
    startWidth->setValue(sw);
    endWidth->setValue(ew);
    _bulge = bulge;
}

lc::geo::Coordinate LWVertexGroup::location() const {
    return coordgui->value();
}
