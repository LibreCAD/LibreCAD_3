#include "listener.h"

#include <QDebug>

#include "cad/base/cadentity.h"

Listener::Listener(lc::AbstractDocument* _document) : QObject() {
    connect(_document, SIGNAL(addEntityEvent(lc::AddEntityEvent*)),
            this, SLOT(on_addEntityEvent(lc::AddEntityEvent*)));

}


void Listener::on_addEntityEvent(const lc::AddEntityEvent& event) {
    //  qDebug() << "Entity with ID: " << event.entity()->id() << " was added.";
}
