#pragma once

#include <string>
#include <memory>

/**
  * If a object is a meta object, eg something that can be attached to a entity then this object must extend MetaType
  * and provide a unique meta name.
  * meta types can also be put into a variant so it can be stored in a map
  */
namespace lc {
    class MetaType {
        public:
            MetaType() {
            }

        public:
            /**
            * Each MetaType to be stored in a entity or later must have a name so it can be looked up
            * This fmethod should return a unique name for each meta name
            * All meta type names internally used will store with a _
            */
            virtual std::string lcMetaName() const {
                return "";
            }
            virtual ~MetaType() {}
    };
    using MetaType_SPtr = std::shared_ptr<MetaType>;
    using MetaType_CSPtr = std::shared_ptr<const MetaType>;

}
