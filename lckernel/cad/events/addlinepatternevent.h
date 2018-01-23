#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * \brief Event that gets emitted when a new line pattern was added to the document
     * \param DxfLinePatternByValue_CSPtr
     */
    class AddLinePatternEvent {
    public:
        /*!
         * \brief Construct an Add Line Pattern Event
         * \param DxfLinePatternByValue_CSPtr Linepattern to be added.
         */
        AddLinePatternEvent(const DxfLinePatternByValue_CSPtr linePattern) : _linePattern(linePattern) {
        }
        /*!
         * \brief Returns the line pattern.
         * \return DxfLinePatternByValue_CSPtr linePattern.
         */
        const DxfLinePatternByValue_CSPtr linePattern() const {
            return _linePattern;
        }

    private:
        const DxfLinePatternByValue_CSPtr _linePattern;
    };
}
