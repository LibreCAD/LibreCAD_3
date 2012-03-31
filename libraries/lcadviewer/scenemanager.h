#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QObject>

#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/document/abstractdocument.h"
#include "lcadviewer.h"

class SceneManager: public QObject {
        Q_OBJECT
    public:
        SceneManager(LCADViewer* viewer, lc::AbstractDocument* _document);

    public slots:
        void on_addEntityEvent(lc::AddEntityEvent* event);
        void on_removeEntityEvent(lc::RemoveEntityEvent* event);

    private:
        lc::AbstractDocument* _document;
        LCADViewer* _viewer;

};

#endif // SCENEMANAGER_H
