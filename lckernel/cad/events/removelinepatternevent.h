#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * \brief Event that gets emited when a new line pattern was removed from the document
     * \param DxfLinePatternByValue_CSPtr
     */
    class RemoveLinePatternEvent {
    public:
        /**
        * \brief Remove Line Pattern Event
        * \param DxfLinePatternByValue_CSPtr Linepattern to be removed.
        */
        RemoveLinePatternEvent(const DxfLinePatternByValue_CSPtr linePattern) : _linePattern(linePattern) {
        }

        /*!
         * \brief Return linePattern data
         * \return LinePattern
         */
        const DxfLinePatternByValue_CSPtr linePattern() const {
            return _linePattern;
        }

    private:
        const DxfLinePatternByValue_CSPtr _linePattern;
    };
}
