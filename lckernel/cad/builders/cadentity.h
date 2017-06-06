#pragma once

#include <cad/meta/layer.h>
#include <cad/base/metainfo.h>
#include <cad/meta/block.h>
#include <cad/base/visitor.h>

namespace lc {
    namespace builder {
        template<typename T>
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
                T* setLayer(const Layer_CSPtr& layer) {
                        _layer = layer;
                        return (T*) this;
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
                T* setMetaInfo(const MetaInfo_CSPtr& metaInfo) {
                        _metaInfo = metaInfo;
                        return (T*) this;
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
                T* setBlock(const Block_CSPtr& block) {
                        _block = block;
                        return (T*) this;
                }

                virtual bool checkValues() {
                        return _layer != nullptr;
                }

            private:
                Layer_CSPtr _layer;
                MetaInfo_CSPtr _metaInfo;
                Block_CSPtr _block;
        };
    }
}