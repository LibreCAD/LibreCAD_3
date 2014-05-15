#ifndef BUILDER_H
#define BUILDER_H


#include "builderops.h"
#include "documentoperation.h"
#include "undoable.h"
#include <QList>
#include "cad/document/storagemanager.h"

namespace lc {
    class Document;
    namespace operation {


        class Builder: public DocumentOperation, public Undoable {
                friend class lc::operation::Base;

            public:
                Builder(Document* document);

                virtual ~Builder();
                Builder& move(const geo::Coordinate& offset);
                Builder& copy(const geo::Coordinate& offset);
                Builder& repeat(const int numTimes);
                Builder& rotate(const geo::Coordinate& rotation_center, const double rotation_angle);
                Builder& scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor);
                Builder& begin();
                Builder& push();
                Builder& remove();
                Builder& selectByLayer(const std::shared_ptr<const Layer>);

                Builder& append(std::shared_ptr<const CADEntity> cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            protected:
                virtual void processInternal(std::shared_ptr<StorageManager> storageManager);

            private:
                QList<std::shared_ptr<Base> > _stack;
                QList<std::shared_ptr<const CADEntity> > _workingBuffer;


                QList<std::shared_ptr<const CADEntity> > _entitiesThatWhereUpdated;
                QList<std::shared_ptr<const CADEntity> > _entitiesThatNeedsRemoval;

        };

    };
}


#endif // BUILDER_H
