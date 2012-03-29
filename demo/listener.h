#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>

#include "cad/events/addentityevent.h"
#include "cad/document/abstractdocument.h"

class Listener : public QObject {
        Q_OBJECT
    public:
        Listener(lc::AbstractDocument* _document);

    public slots:

    public slots:
        void on_addEntityEvent(lc::AddEntityEvent* event);


};

#endif // LISTENER_H
