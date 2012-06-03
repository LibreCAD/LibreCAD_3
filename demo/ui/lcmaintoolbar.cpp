#include "lcmaintoolbar.h"
#include "ui_lcmaintoolbar.h"

#include "../operations/linecreateoperation.h"
#include "../operations/circlecreateoperation.h"
#include "../operations/trimoperation.h"
#include "../operations/operationmanager.h"

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
