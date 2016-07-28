#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * \brief Event that get's emited when a new line pattern was removed from the document
     * \param DxfLinePattern_CSPtr
     */
    class RemoveLinePatternEvent {
    public:
        /**
        * \brief Remove Line Pattern Event
        * \param DxfLinePattern_CSPtr Linepattern to be removed.
        */
        RemoveLinePatternEvent(const DxfLinePattern_CSPtr linePattern) : _linePattern(linePattern) {
        }

        /*!
         * \brief Return linePattern data
         * \return LinePattern
         */
        const DxfLinePattern_CSPtr linePattern() const {
            return _linePattern;
        }

    private:
        const DxfLinePattern_CSPtr _linePattern;
    };
}
