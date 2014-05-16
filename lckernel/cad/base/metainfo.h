#ifndef METAINFO_H
#define METAINFO_H

#include <memory>
#include <map>
#include <set>
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
             * \todo: Change std::vector to a set
             */
            MetaInfo(std::list<MetaType_CSPtr> metaTypes);
            MetaInfo(std::set<MetaType_CSPtr, MetaTypeComp> metaTypes);
            virtual ~MetaInfo();

            MetaType_CSPtr metaType(MetaType::metaTypeId type) const;

            std::set<MetaType_CSPtr, MetaTypeComp> metaTypes() const {
                return _metaTypes;
            }


        private:
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            //std::set<MetaType_CSPtr, MetaTypeComp> _metaTypes;

            std::set<MetaType_CSPtr, MetaTypeComp> _metaTypes;

    };
    typedef std::shared_ptr<MetaInfo> MetaInfo_SPtr;
    typedef std::shared_ptr<const MetaInfo> MetaInfo_CSPtr;


}

#endif // METAINFO_H
