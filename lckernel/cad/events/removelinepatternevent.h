#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
namespace event {
/**
 * \brief Event that gets emitted when a new line pattern was removed from the document
 * \param DxfLinePatternByValue_CSPtr
 */
class RemoveLinePatternEvent {
public:
    /**
    * \brief Remove Line Pattern Event
    * \param DxfLinePatternByValue_CSPtr Linepattern to be removed.
    */
    RemoveLinePatternEvent(const meta::DxfLinePatternByValue_CSPtr linePattern) : _linePattern(linePattern) {
    }

    /*!
     * \brief Return linePattern data
     * \return LinePattern
     */
    const meta::DxfLinePatternByValue_CSPtr linePattern() const {
        return _linePattern;
    }

private:
    const meta::DxfLinePatternByValue_CSPtr _linePattern;
};
}
}
