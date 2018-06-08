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
                        _block(nullptr) {
                }

                virtual ~CADEntityBuilder() = default;

                void copy(entity::CADEntity_CSPtr entity) {
                    _layer = entity->_layer;
                    _block = entity->_block;
                    _metaInfo = entity->_metaInfo;
                    _id.setID(entity->id());
                }

                /**
                 * @brief Get layer
                 * @return Layer
                 */
                const Layer_CSPtr& layer() const {
                        return _layer;
                }

                /**
                 * @brief Set the layer
                 * @param layer New layer
                 */
                void setLayer(const Layer_CSPtr& layer) {
                        _layer = layer;
                }

                /**
                 * @brief Set the layer
                 * @param layer New layer
                 */
                void setLayer(Layer_CSPtr&& layer) {
                    _layer = layer;
                    layer = nullptr;
                }

                /**
                 * @brief Get MetaInfo
                 * @return MetaInfo
                 */
                const MetaInfo_CSPtr& metaInfo() const {
                        return _metaInfo;
                }

                /**
                 * @brief Set MetaInfo
                 * @param metaInfo new MetaInfo
                 */
                void setMetaInfo(const MetaInfo_CSPtr& metaInfo) {
                        _metaInfo = metaInfo;
                }

                /**
                 * @brief Set MetaInfo
                 * @param metaInfo new MetaInfo
                 */
                void setMetaInfo(MetaInfo_CSPtr&& metaInfo) {
                    _metaInfo = metaInfo;
                    metaInfo = nullptr;
                }

                /**
                 * @brief Get block
                 * @return Block
                 */
                const Block_CSPtr& block() const {
                        return _block;
                }

                /**
                 * @brief Set the Block
                 * @param block Block
                 */
                void setBlock(const Block_CSPtr& block) {
                        _block = block;
                }

                /**
                 * @brief Set the Block
                 * @param block Block
                 */
                void setBlock(Block_CSPtr&& block) {
                    _block = block;
                    block = nullptr;
                }

                /**
                 * @brief Get entity ID
                 * @return Entity ID
                 */
                const ID_DATATYPE id() const {
                    return  _id.id();
                }

                /**
                 * @brief Set entity ID
                 * @param id Entity ID
                 */
                void setID(ID_DATATYPE id) {
                    _id.setID(id);
                }

                /**
                 * @brief Generate new ID for the entity
                 */
                void newID() {
                    _id = ID();
                }

                virtual bool checkValues() {
                        return _layer != nullptr;
                }

            private:
                Layer_CSPtr _layer;
                MetaInfo_CSPtr _metaInfo;
                Block_CSPtr _block;
                ID _id;
        };
    }
}