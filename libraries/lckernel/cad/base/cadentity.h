#ifndef CADENTITY_H
#define CADENTITY_H
#include <QList>

#include "cad/const.h"
#include "id.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"
#include "cad/geometry/geoarea.h"

namespace lc {

    class Circle;
    class Line;
    class Arc;
    class Ellipse;
    class CADEntity;

    class EntityInteraction;

    /**
     *Class that all CAD entities must inherit
     *
     */
    class CADEntity : public ID, public MetaInfo {
        public:
            CADEntity();
            CADEntity(QList<std::tr1::shared_ptr<const lc::MetaType> > metaTypes);
            virtual void accept(std::tr1::shared_ptr<const lc::Line>, EntityInteraction&) const = 0;
            virtual void accept(std::tr1::shared_ptr<const lc::Circle>, EntityInteraction&) const = 0;
            virtual void accept(std::tr1::shared_ptr<const lc::Arc>, EntityInteraction&) const = 0;
            virtual void accept(std::tr1::shared_ptr<const lc::Ellipse>, EntityInteraction&) const = 0;
            virtual void accept(std::tr1::shared_ptr<const lc::CADEntity>, EntityInteraction&) const = 0;
    };
}



#endif // CADENTITY_H
