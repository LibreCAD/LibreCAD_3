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



void LCMainToolbar::on_tb_builderTest_clicked() {
    auto layer = _parent->activeMdiChild()->storageManager()->layerByName("0");

    auto l = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(0., 100), layer);

    auto builder = std::make_shared<lc::operation::Builder>(_parent->activeMdiChild()->document());
    builder->append(l);
    builder->execute();

}

void LCMainToolbar::on_tb_snapGrid_clicked() {
    SnapManagerImpl_SPtr sm = std::dynamic_pointer_cast<SnapManagerImpl>(_parent->activeMdiChild()->snapManager());
    if (sm->isGridSnappable()) {
        sm->setGridSnappable(false);
    } else {
        sm->setGridSnappable(true);
    }
}

void LCMainToolbar::on_tb_snapIntersections_clicked() {
    SnapManagerImpl_SPtr sm = std::dynamic_pointer_cast<SnapManagerImpl>(_parent->activeMdiChild()->snapManager());
    if (sm->snapIntersections()) {
        sm->snapIntersections(false);
    } else {
        sm->snapIntersections(true);
    }
}

void LCMainToolbar::on_tb_snapEntity_clicked() {
    auto smi =  std::dynamic_pointer_cast<SnapManagerImpl>(_parent->activeMdiChild()->snapManager());
    lc::SimpleSnapConstrain ssc = smi->snapConstrain();

    if (lc::SimpleSnapConstrain::ON_ENTITY & ssc.constrain()) {
          smi->snapConstrain(ssc.disableConstrain(lc::SimpleSnapConstrain::ON_ENTITY));
    } else {
        smi->snapConstrain(ssc.enableConstrain(lc::SimpleSnapConstrain::ON_ENTITY));
    }
}

void LCMainToolbar::on_tb_snapEntityPath_clicked() {
    auto smi =  std::dynamic_pointer_cast<SnapManagerImpl>(_parent->activeMdiChild()->snapManager());
    lc::SimpleSnapConstrain ssc = smi->snapConstrain();

    if (lc::SimpleSnapConstrain::ON_ENTITYPATH & ssc.constrain()) {
        smi->snapConstrain(ssc.disableConstrain(lc::SimpleSnapConstrain::ON_ENTITYPATH));
    } else {
        smi->snapConstrain(ssc.enableConstrain(lc::SimpleSnapConstrain::ON_ENTITYPATH));
    }
}

void LCMainToolbar::on_tb_snapLogical_clicked() {
    auto smi =  std::dynamic_pointer_cast<SnapManagerImpl>(_parent->activeMdiChild()->snapManager());
    lc::SimpleSnapConstrain ssc = smi->snapConstrain();

    if (lc::SimpleSnapConstrain::LOGICAL & ssc.constrain()) {
        smi->snapConstrain(ssc.disableConstrain(lc::SimpleSnapConstrain::LOGICAL));
    } else {
        smi->snapConstrain(ssc.enableConstrain(lc::SimpleSnapConstrain::LOGICAL));
    }
}
