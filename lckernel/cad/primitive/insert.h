#pragma once

#include <cad/geometry/geocoordinate.h>
#include <cad/base/cadentity.h>
#include <cad/builders/insert.h>
#include <cad/interface/snapable.h>
#include <cad/interface/draggable.h>
#include <cad/document/document.h>

namespace lc {
    namespace entity {
        class Insert : public CADEntity, public std::enable_shared_from_this<Insert>, public Snapable, public Draggable {
            friend class builder::InsertBuilder;

            public:
                Insert(const Insert_CSPtr& other, bool sameID = false);
                ~Insert();

                const Block_CSPtr& displayBlock() const;
                const geo::Coordinate& position() const;
                const Document_SPtr& document() const;

                CADEntity_CSPtr move(const geo::Coordinate& offset) const override;
                CADEntity_CSPtr copy(const geo::Coordinate& offset) const override;
                CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center,
                                       double rotation_angle) const override;
                CADEntity_CSPtr scale(const geo::Coordinate& scale_center,
                                      const geo::Coordinate& scale_factor) const override;
                CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                                       const geo::Coordinate& axis2) const override;
                CADEntity_CSPtr modify(Layer_CSPtr layer,
                                       MetaInfo_CSPtr metaInfo,
                                       Block_CSPtr block) const override;

                const geo::Area boundingBox() const override;

                void dispatch(EntityDispatch& dispatch) const override;

                std::map<unsigned int, geo::Coordinate> dragPoints() const override;
                CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

                std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord,
                                                         const SimpleSnapConstrain& simpleSnapConstrain,
                                                         double minDistanceToSnap,
                                                         int maxNumberOfSnapPoints) const override;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override;

            protected:
                Insert(const builder::InsertBuilder& builder);

            private:
                void calculateBoundingBox();

                void on_addEntityEvent(const lc::AddEntityEvent&);
                void on_removeEntityEvent(const lc::RemoveEntityEvent&);

                Document_SPtr _document;
                geo::Coordinate _position;
                Block_CSPtr _displayBlock;
                geo::Area _boundingBox;
        };

        DECLARE_SHORT_SHARED_PTR(Insert)
    }
}