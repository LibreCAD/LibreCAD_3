#pragma once

#include <unordered_map>
#include <string>
#include "cad/interface/metatype.h"
#include "cad/meta/dxflinepattern.h"

namespace lc {
    namespace meta {
        /**
         * Container to hold meta data for an entity
         */
        // TODO: Container to store meta information on an entity
        class MetaInfo
                : public std::enable_shared_from_this<MetaInfo>,
                  public std::unordered_map<std::string, EntityMetaType_CSPtr> {
            public:
                // Convenience function to add a MetaType to the MetaInfo map
                std::shared_ptr<MetaInfo> add(EntityMetaType_CSPtr mt);

                // std::shared_ptr<MetaInfo> add(std::string name, MetaType_CSPtr mi);

                virtual ~MetaInfo() = default;

                static std::shared_ptr<MetaInfo> create() {
                    return std::make_shared<lc::meta::MetaInfo>();
                }
        };

        DECLARE_SHORT_SHARED_PTR(MetaInfo)
    }
}
