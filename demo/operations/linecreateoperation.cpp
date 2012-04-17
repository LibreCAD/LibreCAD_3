#include "linecreateoperation.h"

LineCreateOperation::LineCreateOperation() : Operation() {
}

void LineCreateOperation::start(QGraphicsView* graphicsView, CursorPtr cursor) {

    connect(graphicsView, SIGNAL(drawEvent(const DrawEvent&)),
            this, SLOT(on_Draw_Event(const DrawEvent&)));

    connect(cursor.get(), SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_mouseReleaseEvent(const MouseReleaseEvent&)));

}

void LineCreateOperation::on_drawEvent(const DrawEvent& event) {

}

void LineCreateOperation::on_mouseReleaseEvent(const MouseReleaseEvent& event) {
    qDebug() << "Mouse release from LineCreateOperation";
}

void LineCreateOperation::undo() {

}

