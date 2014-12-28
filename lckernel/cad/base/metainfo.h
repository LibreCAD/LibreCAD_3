#pragma once

#include <unordered_map>
#include <string>

namespace lc {
    class MetaType;
    typedef std::shared_ptr<const MetaType> MetaType_CSPtr;

    class MetaInfo : public std::unordered_map<std::string, MetaType_CSPtr> {
        public:
            // Used by LibreCAD internally to store a color meta info
            static const std::string _COLOR;
    };


    typedef std::shared_ptr<MetaInfo> MetaInfo_SPtr;
    typedef std::shared_ptr<const MetaInfo> MetaInfo_CSPtr;
}
