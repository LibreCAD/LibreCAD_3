#pragma once

#include <cad/storage/document.h>
#include "cadentity.h"

namespace lc {
    namespace builder {
        class InsertBuilder : public CADEntityBuilder {
            public:
                InsertBuilder();
                virtual ~InsertBuilder() = default;

                InsertBuilder* copy(const entity::Insert_CSPtr& insert);

                const Block_CSPtr& displayBlock() const;
                InsertBuilder* setDisplayBlock(const Block_CSPtr& displayBlock);

                const geo::Coordinate& coordinate() const;
                InsertBuilder* setCoordinate(const geo::Coordinate& coordinate);

                const Document_SPtr& document() const;
                InsertBuilder* setDocument(const Document_SPtr& document);

                bool checkValues() override;
                entity::Insert_CSPtr build();

            private:
                Document_SPtr _document;
                Block_CSPtr _displayBlock;
                geo::Coordinate _coordinate;
        };
    }
}