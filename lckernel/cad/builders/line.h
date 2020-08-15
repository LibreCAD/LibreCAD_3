#pragma once

#include "cadentity.h"

namespace lc {
namespace builder {
class LineBuilder;
class LineBuilder : public CADEntityBuilder {
public:
    LineBuilder();
    virtual ~LineBuilder() = default;

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

    /**
    *  @brief Get angle of line from the horizontal axis
    *  @return angle if line, -1 if entity is not a line
    */
    double getLineAngle(lc::entity::CADEntity_CSPtr entity) const;

    void copy(entity::Line_CSPtr entity);

private:
    geo::Coordinate _start;
    geo::Coordinate _end;
};
}
}
