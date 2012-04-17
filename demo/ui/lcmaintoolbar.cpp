#include "lcmaintoolbar.h"
#include "ui_lcmaintoolbar.h"

#include "../operations/linecreateoperation.h"

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
    LineCreateOperation* lo = new LineCreateOperation();
    lo->start(_parent->activeMdiChild()->view(), _parent->activeMdiChild()->cursor());

}
