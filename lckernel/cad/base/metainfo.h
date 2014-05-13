#ifndef METAINFO_H
#define METAINFO_H

#include <memory>
#include <QHash>
#include <QVariant>
#include "cad/const.h"

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
            MetaInfo(QList<std::shared_ptr<const MetaType> > metaTypes);
            MetaInfo(QHash<int, std::shared_ptr<const MetaType> > metaTypes);
            virtual ~MetaInfo();

            std::shared_ptr<const MetaType> metaType(MetaType::metaTypeId type) const;

            QHash<int, std::shared_ptr<const MetaType> >  metaTypes() const {
                return _metaTypes;
            }

        private:
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            QHash<int, std::shared_ptr<const MetaType> > _metaTypes;
    };
}
#endif // METAINFO_H
