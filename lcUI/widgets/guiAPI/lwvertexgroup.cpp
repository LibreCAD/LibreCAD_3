#include "lwvertexgroup.h"

using namespace lc::ui::api;

LWVertexGroup::LWVertexGroup(std::string label, QWidget* parent)
    :
    HorizontalGroupGUI(label, true, parent)
{
}

void LWVertexGroup::addCallback(kaguya::LuaRef cb) {
    coordgui->addFinishCallback(cb);
    startWidth->addCallback(cb);
    endWidth->addCallback(cb);
}

void LWVertexGroup::setMainWindow(lc::ui::MainWindow* mainWindowIn) {
    coordgui->enableCoordinateSelection(mainWindowIn);
}

void LWVertexGroup::getLuaValue(kaguya::LuaRef& table) {
    HorizontalGroupGUI::getLuaValue(table);
    table[_key] = kaguya::NewTable();
    std::cout << "Creating table for key " << _key << std::endl;

    table[_key][_key + "Location"] = table[_key + "Location"];
    std::cout << "Adding location value " << table[_key + "Location"].get<lc::geo::Coordinate>();
    table[_key][_key + "StartWidth"] = table[_key + "StartWidth"];
    std::cout << "Adding start width value " << table[_key + "StartWidth"].get<double>();
    table[_key][_key + "EndWidth"] = table[_key + "EndWidth"];
}

void LWVertexGroup::setKey(const std::string& keyIn) {
    InputGUI::setKey(keyIn);

    coordgui = new lc::ui::api::CoordinateGUI("Location");
    startWidth = new lc::ui::api::NumberGUI("Start Width");
    endWidth = new lc::ui::api::NumberGUI("End Width");

    addWidget(_key + "Location", coordgui);
    addWidget(_key + "StartWidth", startWidth);
    addWidget(_key + "EndWidth", endWidth);
}
