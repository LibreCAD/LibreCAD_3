#include "entitymanagerimpl.h"

#include <QDebug>
#include "cad/base/cadentity.h"
#include "cad/document/documentlayer.h"

using namespace lc;

EntityManagerImpl::EntityManagerImpl(AbstractDocument* document) : EntityManager(), _document(document) {
    connect(document, SIGNAL(addEntityEvent(const lc::AddEntityEvent&)),
            this, SLOT(on_addEntityEvent(const lc::AddEntityEvent&)));
    connect(document, SIGNAL(removeEntityEvent(const lc::RemoveEntityEvent&)),
            this, SLOT(on_removeEntityEvent(const lc::RemoveEntityEvent&)));
}


void EntityManagerImpl::on_addEntityEvent(const lc::AddEntityEvent& event) {
    // Note: This might seem to be slow, but inserting 1000000 entities took 300ms to insert
    std::tr1::shared_ptr<lc::DocumentLayer> documentLayer = document()->layerManager()->layer(event.layerName());

    documentLayer->addEntity(event.entity());
}

void EntityManagerImpl::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {

    // Removeal of entities currently needs to go over all layers
    // FOr 10.000 entities this takes a couple of seconds so this must be optimzed
    QHash <QString, std::tr1::shared_ptr<lc::DocumentLayer> > allLayers = document()->layerManager()->allLayers();
    QHashIterator<QString, std::tr1::shared_ptr<lc::DocumentLayer> > li(allLayers);

    while (li.hasNext()) {
        li.next();
        std::tr1::shared_ptr<lc::DocumentLayer> documentLayer = li.value();

        try {
            documentLayer->removeEntity(event.id());
        } catch (QString e) {

        }
    }
}

