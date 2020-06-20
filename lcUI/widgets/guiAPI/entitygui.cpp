#include "entitygui.h"
#include "ui_entitygui.h"

using namespace lc::ui::api;

EntityGUI::EntityGUI(std::string label, QWidget* parent)
    :
    InputGUI(label, parent),
    ui(new Ui::EntityGUI)
{
    ui->setupUi(this);
}

EntityGUI::~EntityGUI()
{
    delete ui;
}

void EntityGUI::getLuaValue(kaguya::LuaRef& table) {
    std::cout << "Phew" << std::endl;
}
