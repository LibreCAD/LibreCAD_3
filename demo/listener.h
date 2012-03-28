#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>

#include "cad/events/addentityevent.h"
#include "cad/document/document.h"

class Listener : public QObject
{
    Q_OBJECT
public:
    Listener(lc::Document * _document);

public slots:

public slots:
    void on_addEntityEvent(lc::AddEntityEvent * event);


};

#endif // LISTENER_H
