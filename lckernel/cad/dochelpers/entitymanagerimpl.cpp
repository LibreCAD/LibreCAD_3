#include "entitymanagerimpl.h"

#include <QDebug>
#include <QObject>

#include "cad/base/cadentity.h"

using namespace lc;

EntityManagerImpl::EntityManagerImpl(Document* document) : EntityManager(), _document(document) {
    connect(document, SIGNAL(addEntityEvent(const lc::AddEntityEvent&)),
            this, SLOT(on_addEntityEvent(const lc::AddEntityEvent&)));
    connect(document, SIGNAL(removeEntityEvent(const lc::RemoveEntityEvent&)),
            this, SLOT(on_removeEntityEvent(const lc::RemoveEntityEvent&)));
    connect(document, SIGNAL(replaceEntityEvent(const lc::ReplaceEntityEvent&)),
            this, SLOT(on_replaceEntityEvent(const lc::ReplaceEntityEvent&)));
}


void EntityManagerImpl::on_addEntityEvent(const AddEntityEvent& event) {
    _entities.addEntity(event.entity());
}

void EntityManagerImpl::on_replaceEntityEvent(const ReplaceEntityEvent& event) {
    _entities.replaceEntity(event.entity());
}

void EntityManagerImpl::on_removeEntityEvent(const RemoveEntityEvent& event) {
    _entities.removeEntity(event.entity());
}

shared_ptr<const CADEntity> EntityManagerImpl::findEntityByID(ID_DATATYPE id) const {
    return _entities.findEntityByID(id);
}



