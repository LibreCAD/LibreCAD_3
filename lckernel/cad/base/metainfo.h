#pragma once

#include <unordered_map>
#include <string>
#include "cad/interface/metatype.h"

namespace lc {
    /**
     * Containe to hold meta data for a entity
     */
    // TODO: Container to store meta information on a entity
    class MetaInfo : public std::enable_shared_from_this<MetaInfo>, public std::unordered_map<std::string, EntityMetaType_CSPtr> {
        public:
            // COnvenience function to add a MetaType to the MetaInfo map
            std::shared_ptr<MetaInfo> add(EntityMetaType_CSPtr mi);
            // std::shared_ptr<MetaInfo> add(std::string name, MetaType_CSPtr mi);

            virtual ~MetaInfo() = default;

            static std::shared_ptr<MetaInfo> create() {
                return std::make_shared<lc::MetaInfo>();
            }
    };


    using MetaInfo_SPtr = std::shared_ptr<MetaInfo>;
    using MetaInfo_CSPtr = std::shared_ptr<const MetaInfo>;
}
