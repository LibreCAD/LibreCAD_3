#pragma once

#include <memory>
#include <cad/const.h>
#include <cad/base/cadentity.h>

namespace lc {
    namespace geo {
        class Area;
    }


    namespace viewer {
        class LcDrawOptions;
        class LcPainter;

        /**
    * LCVDrawItem is a abstract class that any class needs to implement if it wants to draw an entity on backgrounds or foregrounds
    * For other objects (Cursor, ...) see files in drawables folder
    * LCVDrawItem is a child of CADEntity as it's the only way to store it in EntityContainer
    */
        class LCVDrawItem {
            public:
                LCVDrawItem(const lc::entity::CADEntity_CSPtr& entity, bool selectable);
                virtual ~LCVDrawItem() = default;

                /**
                * Draw function to draw the final object on a canvas
                * Be aware that thus function must be created thread save
                */
                virtual void draw(LcPainter& _painter, const LcDrawOptions &options, const lc::geo::Area& updateRect) const {
                    // Implement's nothing
                }

                bool selectable() const;
                bool selected() const;

                virtual void selected(bool selected);

                /**
                 * @brief Return the entity which is drawn
                 * @return Entity
                 */
                virtual lc::entity::CADEntity_CSPtr entity() const = 0;

            private:
                bool _selectable;
                bool _selected;
        };

        DECLARE_SHORT_SHARED_PTR(LCVDrawItem)
    }
}
