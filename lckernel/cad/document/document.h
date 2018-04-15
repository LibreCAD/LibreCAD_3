#pragma once

#include "cad/meta/layer.h"

#include "cad/const.h"
#include "cad/dochelpers/entitycontainer.h"
#include "cad/document/storagemanager.h"

#include <nano-signal-slot/nano_signal_slot.hpp>

#include <cad/events/addlayerevent.h>
#include <cad/events/removelayerevent.h>
#include <cad/events/replacelayerevent.h>
#include <cad/events/newwaitingcustomentityevent.h>
#include <unordered_set>
#include "cad/meta/dxflinepattern.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"

#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

#include "cad/events/addlinepatternevent.h"
#include "cad/events/removelinepatternevent.h"
#include "cad/events/replacelinepatternevent.h"

namespace lc {

    namespace entity {
        class CADEntity;
    }

    namespace operation {
        class DocumentOperation;
        DECLARE_SHORT_SHARED_PTR(DocumentOperation)
    }

    class Document {
        public:
            Document();
            virtual ~Document();

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
        
            /*!
             * \brief Event to remove a line pattern
             */
            virtual  Nano::Signal<void(const lc::RemoveLinePatternEvent&)>& removeLinePatternEvent();

            /*!
             * \brief Event to add a line pattern
             */
            virtual  Nano::Signal<void(const lc::AddLinePatternEvent&)>& addLinePatternEvent();

            /*!
             * \brief Event to replace a line pattern
             */
            virtual  Nano::Signal<void(const lc::ReplaceLinePatternEvent&)>& replaceLinePatternEvent();

            /**
             * @brief Event called when an unmanaged entity is added to the document
             */
            virtual Nano::Signal<void(const NewWaitingCustomEntityEvent&)>& newWaitingCustomEntityEvent();

        protected:
            /*!
                 * \brief execute
                 * the given operations within the document
                 * \param operation
                 */
            virtual void execute(const operation::DocumentOperation_SPtr& operation) = 0;

            /*!
             * \brief begins an operation
             * \param operation
             */
            virtual void begin(const operation::DocumentOperation_SPtr& operation) = 0;

            /*!
                 * \brief commits an operation
                 * \param operation
                 */
            virtual void commit(const operation::DocumentOperation_SPtr& operation) = 0;


            /*!
             * \brief operationStart Starts the operation.
             * \param operation
             */
            virtual void operationStart(const operation::DocumentOperation_SPtr& operation);

            /*!
             * \brief operationFinish Finishes the operation.
             * \param operation
             */
            virtual void operationFinish(const operation::DocumentOperation_SPtr& operation);

            /*!
             * \brief operationProcess process the operation.
             * \param operation
             */
            virtual void operationProcess(const operation::DocumentOperation_SPtr& operation);

        public:
            /*!
             * \brief add an entity to document.
             * \param layerName Name of layer to which entity is to be added
             * \param cadEntity Entity to be added
             */
            virtual void insertEntity(const entity::CADEntity_CSPtr& cadEntity) = 0;
            /*!
             * \brief removes an entity from the document.
             * \param id ID of the entity to be removed.
             */
            virtual void removeEntity(const entity::CADEntity_CSPtr& entity) = 0;

            /**
            *  \brief add a new layer to the document
            *  \param layer layer to be added.
            */
            virtual void addDocumentMetaType(const DocumentMetaType_CSPtr& dmt) = 0;

            /**
            *  \brief remove a layer from the document
            *  \param layer layer to be added.
            */
            virtual void removeDocumentMetaType(const DocumentMetaType_CSPtr& dmt) = 0;

            /**
            *  \brief remove a layer from the document
            */
            virtual void replaceDocumentMetaType(const DocumentMetaType_CSPtr& oldDmt, DocumentMetaType_CSPtr newDmt) = 0;

            /*!
             * \brief findEntitiesByLayer
             * rturn a entities container with all entities on the given layer
             * \param layer
             * \return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entitiesByLayer(const Layer_CSPtr& layer) = 0;

            /**
             * @brief Search entities in a given block
             * @param block
             * @return EntityContainer
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entitiesByBlock(const Block_CSPtr& block) = 0;

            /**
             * @brief entityContainer
             * Return a copy of all entities within the document
             * @return
             */
            virtual EntityContainer<entity::CADEntity_CSPtr> entityContainer() = 0;


            /**
             * @brief Returns all layers
             */
            virtual std::map<std::string, Layer_CSPtr> allLayers() const = 0;

            virtual std::map<std::string, DocumentMetaType_CSPtr, lc::StringHelper::cmpCaseInsensetive> allMetaTypes() = 0;

            /**
             * @brief return a layer by it's name
             */
            virtual Layer_CSPtr layerByName(const std::string& layerName) const = 0;

            /**
             * @brief return a line pattern by it's name
             */
            virtual DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const = 0;

            /**
             * @brief return all line patterns
             */
            virtual std::vector<DxfLinePatternByValue_CSPtr> linePatterns() const = 0;

            /**
             * @return all blocks
             */
            virtual std::vector<Block_CSPtr> blocks() const = 0;

            /**
             * @return all the custom entities which aren't managed by a plugin
             */
            virtual std::unordered_set<entity::Insert_CSPtr> waitingCustomEntities(const std::string& pluginName) = 0;

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

            Nano::Signal<void(const lc::AddLinePatternEvent&)>  _addLinePatternEvent;
            Nano::Signal<void(const lc::ReplaceLinePatternEvent&)>  _replaceLinePatternEvent;
            Nano::Signal<void(const lc::RemoveLinePatternEvent&)>  _removeLinePatternEvent;

            Nano::Signal<void(const lc::NewWaitingCustomEntityEvent&)> _newWaitingCustomEntityEvent;
    };

    DECLARE_SHORT_SHARED_PTR(Document);
}