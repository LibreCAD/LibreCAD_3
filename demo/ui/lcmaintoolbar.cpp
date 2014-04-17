#include "lcmaintoolbar.h"
#include "ui_lcmaintoolbar.h"

#include "../operations/linecreateoperation.h"
#include "../operations/circlecreateoperation.h"
#include "../operations/trimoperation.h"
#include "../operations/operationmanager.h"

#include <cad/operations/builder.h>
#include <cad/operations/create.h>

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
    shared_ptr<GuiOperation> lo = shared_ptr<GuiOperation>(new LineCreateOperation(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->view(), _parent->activeMdiChild()->snapManager()));
    _parent->activeMdiChild()->operationManager()->startOperation(lo);
}

void LCMainToolbar::on_toolButton_2_clicked() {
    shared_ptr<GuiOperation> lo = shared_ptr<GuiOperation>(new CircleCreateOperation(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->view(), _parent->activeMdiChild()->snapManager()));
    _parent->activeMdiChild()->operationManager()->startOperation(lo);
}

void LCMainToolbar::on_toolButton_3_clicked()
{
    shared_ptr<GuiOperation> lo = shared_ptr<GuiOperation>(new TrimOperation(_parent->activeMdiChild()->document(), _parent->activeMdiChild()->view(), _parent->activeMdiChild()->snapManager(), _parent->activeMdiChild()->selectionManager()));
    _parent->activeMdiChild()->operationManager()->startOperation(lo);

}

void LCMainToolbar::on_toolButton_4_clicked()
{
    if (_parent->activeMdiChild()->snapManager()->isGridSnappable()) {
        _parent->activeMdiChild()->snapManager()->setGridSnappable(false);
    } else {
        _parent->activeMdiChild()->snapManager()->setGridSnappable(true);
    }

}

void LCMainToolbar::on_toolButton_5_clicked()
{
    auto l = make_shared<const lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(0., 100));
    auto foo = make_shared<lc::operation::Create>(_parent->activeMdiChild()->document(), "0");
    foo->append(l);
    foo->execute();

    auto b = make_shared<lc::operation::Builder>(_parent->activeMdiChild()->document());
    b->append(l).copy(lc::geo::Coordinate(10., 0.)).repeat(10).move(lc::geo::Coordinate(50., 50));
    b->execute();


}
