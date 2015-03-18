#pragma once

#include <unordered_map>
#include <string>
#include "cad/interface/metatype.h"

namespace lc {
    class MetaInfo : public std::enable_shared_from_this<MetaInfo>, public std::unordered_map<std::string, MetaType_CSPtr> {
        public:
            // COnvenience function to add a MetaType to the MetaInfo map
            std::shared_ptr<MetaInfo> add(MetaType_CSPtr mi);
            // std::shared_ptr<MetaInfo> add(std::string name, MetaType_CSPtr mi);

            virtual ~MetaInfo() = default;

            static std::shared_ptr<MetaInfo> create() {
                return std::make_shared<lc::MetaInfo>();
            }
    };


    typedef std::shared_ptr<MetaInfo> MetaInfo_SPtr;
    typedef std::shared_ptr<const MetaInfo> MetaInfo_CSPtr;
}
