#include "entitymanagerimpl.h"

#include <QDebug>
#include "cad/base/cadentity.h"
#include "cad/document/documentlayer.h"

using namespace lc;


void EntityManagerImpl::document(AbstractDocument* document) {
    EntityManager::document(document);
    connect(document, SIGNAL(addEntityEvent(lc::AddEntityEvent*)),
            this, SLOT(on_addEntityEvent(lc::AddEntityEvent*)));

}

void EntityManagerImpl::on_addEntityEvent(lc::AddEntityEvent* event) {
    // FIXME: Speedup location of DocumentLayer by a local cache? THis can be done since we properly import load sof entities into the same layer
    DocumentLayer* documentLayer = Document()->layerManager()->layer(event->layerName());

    documentLayer->addEntity(event->entity());
    //    qDebug() << "EntityManagerImpl with ID: " << event->entity()->id() << " was added.";
}
