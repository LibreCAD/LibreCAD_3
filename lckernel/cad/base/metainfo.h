#ifndef METAINFO_H
#define METAINFO_H

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
             */
            MetaInfo(QList<shared_ptr<const MetaType> > metaTypes);
            virtual ~MetaInfo();

            shared_ptr<const MetaType> metaType(MetaType::metaTypeId type) const;


            QHash<int, shared_ptr<const MetaType> >  getMetaTypes() const {
                return _metaTypes;
            }

        private:
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            QHash<int, shared_ptr<const MetaType> > _metaTypes;
    };
}
#endif // METAINFO_H
