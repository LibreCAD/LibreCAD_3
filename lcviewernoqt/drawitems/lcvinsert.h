#pragma once

#include <cad/primitive/insert.h>
#include <cad/dochelpers/entitycontainer.h>
#include <cad/storage/document.h>
#include <unordered_set>
#include "lcvdrawitem.h"
#include "../documentcanvas.h"

namespace LCViewer {
    class LCVInsert : public LCVDrawItem {
        public:
            LCVInsert(lc::entity::Insert_CSPtr& insert);
            virtual ~LCVInsert();

            void selected(bool selected) override;

            void draw(LcPainter& _painter, const LcDrawOptions& options, const lc::geo::Area& updateRect) const override;
            void draw(const DocumentCanvas_SPtr& docCanvas, LcPainter& painter) const;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            void append(const lc::entity::CADEntity_CSPtr& entity);

            void on_addEntityEvent(const lc::AddEntityEvent&);
            void on_removeEntityEvent(const lc::RemoveEntityEvent&);

        private:
            lc::entity::Insert_CSPtr _insert;
            lc::geo::Coordinate _offset;
            std::map<ID_DATATYPE, LCVDrawItem_SPtr> _entities;
    };
}