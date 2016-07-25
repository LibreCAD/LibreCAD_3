#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * \brief Event that get's emited when a new line pattern was added to the document
     * \param DxfLinePattern_CSPtr
     */
    class AddLinePatternEvent {
    public:
        /*!
         * \brief Construct an Add Line Pattern Event
         * \param DxfLinePattern_CSPtr Linepattern to be added.
         */
        AddLinePatternEvent(const DxfLinePattern_CSPtr linePattern) : _linePattern(linePattern) {
        }
        /*!
         * \brief Returns the line pattern.
         * \return DxfLinePattern_CSPtr linePattern.
         */
        const DxfLinePattern_CSPtr linePattern() const {
            return _linePattern;
        }

    private:
        const DxfLinePattern_CSPtr _linePattern;
    };
}
