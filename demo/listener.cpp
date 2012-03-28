#include "listener.h"

#include <QDebug>

#include "cad/base/cadentity.h"

Listener::Listener(lc::Document * _document) : QObject(_document)
{
    connect(_document, SIGNAL(addEntityEvent(lc::AddEntityEvent  *)),
                         this, SLOT(on_addEntityEvent(lc::AddEntityEvent *)));

}


void Listener::on_addEntityEvent(lc::AddEntityEvent * event)
{
    qDebug() << "Entity with ID: " << event->entity()->id() << " was added.";
}
