#pragma once

#include <cad/primitive/insert.h>
#include <cad/dochelpers/entitycontainer.h>
#include <cad/document/document.h>
#include <unordered_set>
#include "lcvdrawitem.h"
#include "../documentcanvas.h"

namespace LCViewer {
    class LCVInsert : public lc::entity::Insert, public LCVDrawItem {
        public:
            LCVInsert(lc::entity::Insert_CSPtr insert);
            virtual ~LCVInsert();

            virtual void selected(bool selected) override;

            void draw(LcPainter& _painter, const LcDrawOptions& options, const lc::geo::Area& updateRect) const override;
            void draw(DocumentCanvas_SPtr docCanvas) const;

        private:
            void append(lc::entity::CADEntity_CSPtr entity);

            void on_addEntityEvent(const lc::AddEntityEvent&);
            void on_removeEntityEvent(const lc::RemoveEntityEvent&);

        private:
            lc::geo::Coordinate _offset;
            std::map<ID_DATATYPE, LCVDrawItem_SPtr> _entities;
    };
}