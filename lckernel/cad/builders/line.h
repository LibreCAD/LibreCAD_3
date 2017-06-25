#pragma once

#include "cadentity.h"

namespace lc {
    namespace builder {
        class LineBuilder;
        class LineBuilder : public CADEntityBuilder<LineBuilder> {
            public:
                LineBuilder();

                /**
                 * @brief Get line start
                 * @return Coordinate
                 */
                const geo::Coordinate& start() const;

                /**
                 * @brief Set line start
                 * @param start Coordinate
                 */
                LineBuilder* setStart(const geo::Coordinate& start);

                /**
                 * @brief Get line end
                 * @return Coordinate
                 */
                const geo::Coordinate& end() const;

                /**
                 * @brief Set line end
                 * @param end Coordinate
                 */
                LineBuilder* setEnd(const geo::Coordinate& end);

                /**
                 * @brief Create the line
                 * @return Line
                 */
                entity::Line_CSPtr build();

            private:
                geo::Coordinate _start;
                geo::Coordinate _end;
        };
    }
}