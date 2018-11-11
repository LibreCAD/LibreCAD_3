#pragma once

#include <cad/base/cadentity.h>

namespace lc {
    namespace builder {
        class CADEntityBuilder {
            public:
                /**
                 * Create a new builder for CADEntity
                 */
                CADEntityBuilder() :
                        _layer(nullptr),
                        _metaInfo(nullptr),
                        _block(nullptr),
                        _id(nullptr) {
                }

                virtual ~CADEntityBuilder() {
                    delete _id;
                };

                void copy(entity::CADEntity_CSPtr entity) {
                    _layer = entity->_layer;
                    _block = entity->_block;
                    _metaInfo = entity->_metaInfo;
                    _id = new entity::ID(entity->id());
                }

                /**
                 * @brief Get layer
                 * @return Layer
                 */
                const meta::Layer_CSPtr& layer() const {
                        return _layer;
                }

                /**
                 * @brief Set the layer
                 * @param layer New layer
                 */
                void setLayer(const meta::Layer_CSPtr& layer) {
                        _layer = layer;
                }

                /**
                 * @brief Set the layer
                 * @param layer New layer
                 */
                void setLayer(meta::Layer_CSPtr&& layer) {
                    _layer = layer;
                    layer = nullptr;
                }

                /**
                 * @brief Get MetaInfo
                 * @return MetaInfo
                 */
                const meta::MetaInfo_CSPtr& metaInfo() const {
                        return _metaInfo;
                }

                /**
                 * @brief Set MetaInfo
                 * @param metaInfo new MetaInfo
                 */
                void setMetaInfo(const meta::MetaInfo_CSPtr& metaInfo) {
                        _metaInfo = metaInfo;
                }

                /**
                 * @brief Set MetaInfo
                 * @param metaInfo new MetaInfo
                 */
                void setMetaInfo(meta::MetaInfo_CSPtr&& metaInfo) {
                    _metaInfo = metaInfo;
                    metaInfo = nullptr;
                }

                /**
                 * @brief Get block
                 * @return Block
                 */
                const meta::Block_CSPtr& block() const {
                        return _block;
                }

                /**
                 * @brief Set the Block
                 * @param block Block
                 */
                void setBlock(const meta::Block_CSPtr& block) {
                        _block = block;
                }

                /**
                 * @brief Set the Block
                 * @param block Block
                 */
                void setBlock(meta::Block_CSPtr&& block) {
                    _block = block;
                    block = nullptr;
                }

                /**
                 * @brief Get entity ID
                 * @return Entity ID
                 */
                const ID_DATATYPE id() const {
                    if(_id == nullptr) {
                        _id = new entity::ID();
                    }

                    return  _id.id();
                }

                /**
                 * @brief Set entity ID
                 * @param id Entity ID
                 */
                void setID(ID_DATATYPE id) {
                    if(_id == nullptr) {
                        _id = new entity::ID(id);
                    }
                    else {
                        _id.setID(id);
                    }
                }

                /**
                 * @brief Generate new ID for the entity
                 */
                void newID() {
                    delete _id;
                    _id = new entity::ID();
                }

                virtual bool checkValues() {
                    return _layer != nullptr;
                }

            private:
                meta::Layer_CSPtr _layer;
                meta::MetaInfo_CSPtr _metaInfo;
                meta::Block_CSPtr _block;
                entity::ID* _id;
        };
    }
}