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
                Insert(Insert_CSPtr other, bool sameID = false);

                const Block_CSPtr& displayBlock() const;
                const geo::Coordinate& position() const;
                const Document_SPtr& document() const;

                CADEntity_CSPtr move(const geo::Coordinate& offset) const override;
                CADEntity_CSPtr copy(const geo::Coordinate& offset) const override;
                CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const override;
                CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const override;
                CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;
                CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

                const geo::Area boundingBox() const override;

                void dispatch(EntityDispatch& dispatch) const override;

                std::map<unsigned int, geo::Coordinate> dragPoints() const override;
                CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

                std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain& simpleSnapConstrain,
                           double minDistanceToSnap, int maxNumberOfSnapPoints) const override;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override;

            private:
                Insert(const builder::InsertBuilder& builder);

                Document_SPtr _document;
                geo::Coordinate _position;
                Block_CSPtr _displayBlock;
        };

        using Insert_SPtr = std::shared_ptr<Insert>;
        using Insert_CSPtr = std::shared_ptr<const Insert>;
    }
}