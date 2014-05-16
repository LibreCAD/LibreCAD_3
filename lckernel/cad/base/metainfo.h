#ifndef METAINFO_H
#define METAINFO_H

#include <memory>
#include <map>
#include <list>
#include "cad/interface/metatype.h"

namespace lc {

    class MetaInfo {
        public:
            MetaInfo();
            /*!
             * \brief Construct a new metainfo
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             *
             * \todo: Change QList to a set
             */
            MetaInfo(std::list<MetaType_CSPtr> metaTypes);
            MetaInfo(std::map<MetaType::metaTypeId, MetaType_CSPtr> metaTypes);
            virtual ~MetaInfo();

            MetaType_CSPtr metaType(MetaType::metaTypeId type) const;

            std::map<MetaType::metaTypeId, MetaType_CSPtr> metaTypes() const {
                return _metaTypes;
            }


        private:
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            std::map<MetaType::metaTypeId, MetaType_CSPtr> _metaTypes;
    };
    typedef std::shared_ptr<MetaInfo> MetaInfo_SPtr;
    typedef std::shared_ptr<const MetaInfo> MetaInfo_CSPtr;

}

#endif // METAINFO_H
