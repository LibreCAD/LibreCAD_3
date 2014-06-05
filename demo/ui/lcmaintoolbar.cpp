#include "lcmaintoolbar.h"
#include "ui_lcmaintoolbar.h"

#include "../operations/linecreateoperation.h"
#include "../operations/circlecreateoperation.h"
#include "../operations/trimoperation.h"
#include "../operations/operationmanager.h"

#include <cad/operations/builder.h>

LCMainToolbar::LCMainToolbar(IMainWindow* parent) :
    QWidget(parent),
    ui(new Ui::LCMainToolbar),
    _parent(parent) {
    ui->setupUi(this);
}

LCMainToolbar::~LCMainToolbar() {
    delete ui;
}

void LCMainToolbar::on_toolButton_clicked() {
    auto layer = _parent->activeMdiChild()->storageManager()->layerByName("0");
    //GuiOperation_SPtr lo = GuiOperation_SPtr(new LineCreateOperation(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->storageManager(), layer, _parent->activeMdiChild()->view(), _parent->activeMdiChild()->snapManager()));
    //_parent->activeMdiChild()->operationManager()->startOperation(lo);
}

void LCMainToolbar::on_toolButton_2_clicked() {
    auto layer = _parent->activeMdiChild()->storageManager()->layerByName("0");
     //auto lo = GuiOperation_SPtr(new CircleCreateOperation(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->storageManager(), layer,  _parent->activeMdiChild()->view(), _parent->activeMdiChild()->snapManager()));
     //_parent->activeMdiChild()->operationManager()->startOperation(lo);
}

void LCMainToolbar::on_toolButton_3_clicked() {
    auto layer = _parent->activeMdiChild()->storageManager()->layerByName("0");
    //  GuiOperation_SPtr lo = GuiOperation_SPtr(new TrimOperation(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->storageManager(), _parent->activeMdiChild()->view(), _parent->activeMdiChild()->snapManager(), _parent->activeMdiChild()->selectionManager()));
    //   _parent->activeMdiChild()->operationManager()->startOperation(lo);

}

void LCMainToolbar::on_toolButton_4_clicked() {
    if (_parent->activeMdiChild()->snapManager()->isGridSnappable()) {
        _parent->activeMdiChild()->snapManager()->setGridSnappable(false);
    } else {
        _parent->activeMdiChild()->snapManager()->setGridSnappable(true);
    }

}

void LCMainToolbar::on_toolButton_5_clicked() {
    auto layer = _parent->activeMdiChild()->storageManager()->layerByName("0");

    auto l = std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(0., 100), layer);

    auto builder = std::make_shared<lc::operation::Builder>(_parent->activeMdiChild()->document());
    builder->append(l);
    builder->execute();

}
