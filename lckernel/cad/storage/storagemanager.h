#pragma once

#include "cad/base/id.h"
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"
#include "cad/meta/viewport.h"
#include "cad/storage/entitycontainer.h"
#include <cad/tools/string_helper.h>
#include <map>
#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * Entity manager is responsible for adding/removing entities from a document
     */
    namespace storage {
        class StorageManager {
            public:
                virtual void insertEntity(entity::CADEntity_CSPtr) = 0;

                virtual void insertEntityContainer(const EntityContainer <entity::CADEntity_CSPtr>&) = 0;

                virtual void removeEntity(entity::CADEntity_CSPtr) = 0;

                virtual entity::CADEntity_CSPtr entityByID(ID_DATATYPE id) const = 0;

                virtual EntityContainer <entity::CADEntity_CSPtr> entitiesByLayer(meta::Layer_CSPtr layer) const = 0;
                virtual EntityContainer <entity::CADEntity_CSPtr> entitiesByViewport(meta::Viewport_CSPtr viewport) const = 0;

                virtual EntityContainer <entity::CADEntity_CSPtr> entitiesByBlock(meta::Block_CSPtr block) const = 0;

                /*!
                 * \brief layer
                 * Return a single document layer
                 * \param layerName
                 * \return
                 */
                virtual meta::Layer_CSPtr layerByName(const std::string& layerName) const = 0;

                virtual meta::Viewport_CSPtr viewportByName(const std::string& viewportName) const = 0;

                /*!
                 * \brief DxfLinePattern
                 * Return a line pattern by name from the document
                 * \param linePatternName
                 * \return
                 */
                virtual meta::DxfLinePatternByValue_CSPtr linePatternByName(const std::string& linePatternName) const = 0;

                /*!
                 * \brief Returns all the layers present in the document.
                 * \return Hash Layername, Layer
                 */
                virtual std::map<std::string, meta::Layer_CSPtr> allLayers() const = 0;
                virtual std::map<std::string, meta::Viewport_CSPtr> allViewports() const = 0;

                /*!
                 * \brief entityContainer
                 * return a copy of all entities managed within the storage manager
                 * \return
                 */
                virtual EntityContainer <entity::CADEntity_CSPtr>& entityContainer() = 0;

                /**
                *  \brief add a document meta type
                *  \param layer layer to be added.
                */
                virtual void addDocumentMetaType(meta::DocumentMetaType_CSPtr dmt) = 0;

                /**
                *  \brief remove a document meta type from the document
                *  \param layer layer to be added.
                */
                virtual void removeDocumentMetaType(meta::DocumentMetaType_CSPtr dmt) = 0;

                /**
                *  \brief remove document meta type
                */
                virtual void replaceDocumentMetaType(meta::DocumentMetaType_CSPtr oldDmt, meta::DocumentMetaType_CSPtr newDmt) = 0;

                virtual std::map<std::string, meta::DocumentMetaType_CSPtr, lc::tools::StringHelper::cmpCaseInsensetive>
                allMetaTypes() const = 0;

                /**
                 * @brief optimise
                 * the underlaying data store. Run this at a regular base, for example after each task
                 */
                virtual void optimise() = 0;


                template<typename T>
                const std::shared_ptr<const T> metaDataTypeByName(const std::string& name) const {
                    std::string n = T::LCMETANAME();
                    auto r = _metaDataTypeByName(T::LCMETANAME() + "_" + name);
                    // What do we do when we don't have a meta data set??
                    if (r != nullptr) {
                        std::string myid = r->id();
                        auto dp = std::dynamic_pointer_cast<const T>(r);
                        return dp;
                    }
                    return nullptr;
                };

                template<typename T>
                const std::vector<std::shared_ptr<const T>> metaTypes() const {
                    auto metaTypes = allMetaTypes();
                    std::vector<std::shared_ptr<T>> matches;

                    for (auto metaType : metaTypes) {
                        auto castedMetaType = std::dynamic_pointer_cast<T>(metaType.second);
                        if (castedMetaType) {
                            matches.push_back(castedMetaType);
                        }
                    }

                    return matches;
                };

            private:
                virtual meta::DocumentMetaType_CSPtr _metaDataTypeByName(const std::string& id) const {
                    return nullptr;
                }
        };

        DECLARE_SHORT_SHARED_PTR(StorageManager)
    }
}
