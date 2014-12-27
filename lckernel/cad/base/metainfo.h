#pragma once

#include <memory>
#include <map>
#include <set>
#include <list>
#include "cad/interface/metatype.h"
#include <unordered_map>

namespace lc {

    class MetaInfo : public std::unordered_map<std::string, MetaType_CSPtr> {
        public:
            // Used by LibreCAD internally to store a color meta info
            static const std::string _COLOR;
    };


    typedef std::shared_ptr<MetaInfo> MetaInfo_SPtr;
    typedef std::shared_ptr<const MetaInfo> MetaInfo_CSPtr;
    typedef std::unique_ptr<MetaInfo> MetaInfo_UPtr;
}
