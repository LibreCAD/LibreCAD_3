#ifndef BUILDER_H
#define BUILDER_H


#include "builderops.h"
#include "documentoperation.h"
#include "undoable.h"
#include <vector>
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
                Builder& selectByLayer(const Layer_CSPtr);

                Builder& append(CADEntity_CSPtr cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            protected:
                virtual void processInternal(StorageManager_SPtr storageManager);

            private:
                std::vector<Base_SPtr> _stack;
                std::vector<CADEntity_CSPtr> _workingBuffer;


                std::vector<CADEntity_CSPtr> _entitiesThatWhereUpdated;
                std::vector<CADEntity_CSPtr> _entitiesThatNeedsRemoval;

        };

    };
}


#endif // BUILDER_H
