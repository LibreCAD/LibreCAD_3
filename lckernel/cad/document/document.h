#pragma once

#include "cad/meta/layer.h"

#include "cad/const.h"
#include "cad/dochelpers/entitycontainer.h"
#include "cad/document/storagemanager.h"

#include "nano-signal-slot/nano_signal_slot.hpp"

#include <cad/events/addlayerevent.h>
#include <cad/events/removelayerevent.h>
#include <cad/events/replacelayerevent.h>

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"

#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

namespace lc {

    namespace entity {
        class CADEntity;
    }

    namespace operation {
        class DocumentOperation;
        typedef std::shared_ptr<DocumentOperation> DocumentOperation_SPtr;
        typedef std::shared_ptr<const DocumentOperation> DocumentOperation_CSPtr;
    }

    class Document {

        public:
            /*!
             * \brief begins a Process Event
             */
            virtual  Nano::Signal<void(const lc::BeginProcessEvent&)>& beginProcessEvent();
            /*!
             * \brief commits a Process Event
             */
            virtual  Nano::Signal<void(const lc::CommitProcessEvent&)>& commitProcessEvent();

            /*!
             * \brief Event to add an Entity
             */
            virtual  Nano::Signal<void(const lc::AddEntityEvent&)>& addEntityEvent();

            /*!
             * \brief Event to replace an Entity
             */
            virtual  Nano::Signal<void(const lc::ReplaceEntityEvent&)>& replaceEntityEvent();

            /*!
             * \brief Event to remove an Entity
             */
            virtual  Nano::Signal<void(const lc::RemoveEntityEvent&)>& removeEntityEvent();

            /*!
             * \brief Event to remove an layer
             */
            virtual  Nano::Signal<void(const lc::RemoveLayerEvent&)>& removeLayerEvent();

            /*!
             * \brief Event to add a layer
             */
            virtual  Nano::Signal<void(const lc::AddLayerEvent&)>& addLayerEvent();

            /*!
             * \brief Event to replace a layer
             */
            virtual  Nano::Signal<void(const lc::ReplaceLayerEvent&)>& replaceLayerEvent();

        protected:
            /*!
                 * \brief execute
                 * the given operations within the document
                 * \param operation
                 */
            virtual void execute(operation::DocumentOperation_SPtr operation) = 0;

            /*!
             * \brief begins an operation
             * \param operation
             */
            virtual void begin(operation::DocumentOperation_SPtr operation) = 0;

            /*!
                 * \brief commits an operation
                 * \param operation
                 */
            virtual void commit(operation::DocumentOperation_SPtr operation) = 0;


            /*!
             * \brief operationStart Starts the operation.
             * \param operation
             */
            virtual void operationStart(operation::DocumentOperation_SPtr operation);

            /*!
                 * \brief operationFinnish Finishes the operation.
                 * \param operation
                 */
            virtual void operationFinnish(operation::DocumentOperation_SPtr operation);

            /*!
                 * \brief operationProcess process the operation.
                 * \param operation
                 */
            virtual void operationProcess(operation::DocumentOperation_SPtr operation);

        public:
            /*!
             * \brief add an entity to document.
             * \param layerName Name of layer to which entity is to be added
             * \param cadEntity Entity to be added
             */
            virtual void insertEntity(const entity::CADEntity_CSPtr cadEntity) = 0;
            /*!
             * \brief removes an entity from the document.
             * \param id ID of the entity to be removed.
             */
            virtual void removeEntity(const entity::CADEntity_CSPtr entity) = 0;

            /**
            *  \brief add a new layer to the document
            *  \param layer layer to be added.
            */
            virtual void addLayer(const Layer_CSPtr layer) = 0;

            /**
            *  \brief remove a layer from the document
            *  \param layer layer to be added.
            */
            virtual void removeLayer(const Layer_CSPtr layer) = 0;

            /**
            *  \brief remove a layer from the document
            */
            virtual void replaceLayer(const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer) = 0;

            /*!
             * \brief findEntitiesByLayer
             * rturn a entities container with all entities on the given layer
             * \param layer
             * \return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr layer) = 0;

            /**
             * @brief entityContainer
             * Return a copy of all entities within the document
             * @return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entityContainer() = 0;

            /**
             * @brief return a layer by it's name
             */
            virtual Layer_CSPtr layerByName(const std::string& layerName) const = 0;
        public:
            friend class lc::operation::DocumentOperation;

        private:
            Nano::Signal<void(const lc::BeginProcessEvent&)>  _beginProcessEvent;
            Nano::Signal<void(const lc::CommitProcessEvent&)>  _commitProcessEvent;

            Nano::Signal<void(const lc::AddEntityEvent&)>  _addEntityEvent;
            Nano::Signal<void(const lc::ReplaceEntityEvent&)>  _replaceEntityEvent;
            Nano::Signal<void(const lc::RemoveEntityEvent&)>  _removeEntityEvent;

            Nano::Signal<void(const lc::AddLayerEvent&)>  _addLayerEvent;
            Nano::Signal<void(const lc::ReplaceLayerEvent&)>  _replaceLayerEvent;
            Nano::Signal<void(const lc::RemoveLayerEvent&)>  _removeLayerEvent;

    };



}
