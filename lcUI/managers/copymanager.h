#pragma once

#include <cstddef>
#include <vector>

#include <cad/base/cadentity.h>
#include <cad/geometry/geocoordinate.h>
#include <cadmdichild.h>

namespace lc {
    namespace ui {
        /**
        * \brief Copy Manager
        *
        * Cut, copy and paste of drawing entities.  Entities are immutable, so
        * the clipboard holds the copied entities themselves, and a paste adds
        * copies of them with new IDs.  The clipboard is shared by every window,
        * so what is copied from one drawing can be pasted into another.
        */
        class CopyManager
        {
        public:
            /**
            * \brief CopyManager constructor
            * \param pointer to CadMdiChild
            */
            CopyManager(CadMdiChild* cadMdiChild = nullptr);

            /**
            * \brief Put entities on the clipboard
            * \param cadEntities entities to copy; an empty list leaves the clipboard as it was
            */
            void copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities);

            /**
            * \brief Put entities on the clipboard and remove them from the drawing, as one undo step
            * \param cadEntities entities to cut; an empty list does nothing
            */
            void cutEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities);

            /**
            * \brief The clipboard entities that can be pasted into this window's drawing
            *
            * A block reference names a block of the drawing it was copied from,
            * so it only pastes back into that drawing.
            */
            std::vector<lc::entity::CADEntity_CSPtr> pasteableEntities() const;

            /**
            * \brief The lower left corner of the pasteable entities, which a paste puts where the user clicks
            */
            lc::geo::Coordinate basePoint() const;

            /**
            * \brief Add the pasteable entities to this window's drawing, moved by offset, as one undo step
            *
            * Each entity keeps its layer.  A layer the drawing has no layer of
            * that name for is added to the drawing in the same undo step.
            * \return the number of entities pasted
            */
            std::size_t paste(const lc::geo::Coordinate& offset);

        private:
            CadMdiChild* _cadMdiChild;
        };
    }
}
