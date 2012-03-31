#include "entitymanagerimpl.h"

#include <QDebug>
#include "cad/base/cadentity.h"
#include "cad/document/documentlayer.h"

using namespace lc;


void EntityManagerImpl::setDocument(AbstractDocument* document) {
    EntityManager::setDocument(document);
    connect(document, SIGNAL(addEntityEvent(lc::AddEntityEvent*)),
            this, SLOT(on_addEntityEvent(lc::AddEntityEvent*)));

}

void EntityManagerImpl::on_addEntityEvent(lc::AddEntityEvent* event) {
    // Note: This might seem to be slow, but inserting 1000000 entities took 300ms to insert
    DocumentLayer* documentLayer = document()->layerManager()->layer(event->layerName());
    documentLayer->addEntity(event->entity());
}
