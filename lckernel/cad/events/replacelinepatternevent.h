#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * \brief Event that get's emited when a new line pattern was replaced in the document
     * \param DxfLinePattern_CSPtr
     */
    class ReplaceLinePatternEvent {
    public:
        /**
         * \brief Replace LinePattern Event
         * \param LinePattern_CSPtr old line pattern
         * \param LinePattern_CSPtr new line pattern
         */
        ReplaceLinePatternEvent(const DxfLinePattern_CSPtr oldLinePattern, const DxfLinePattern_CSPtr newLinePattern)
                : _oldLinePattern(oldLinePattern), _newLinePattern(newLinePattern) {
        }

        /*!
         * \brief Return LinePattern to replace
         * \return DxfLinePattern_CSPtr
         */
        const DxfLinePattern_CSPtr oldLinePattern() const {
            return _oldLinePattern;
        }

        /*!
         * \brief Return LinePattern to add
         * \return DxfLinePattern_CSPtr
         */
        const DxfLinePattern_CSPtr newLinePattern() const {
            return _newLinePattern;
        }

    private:
        const DxfLinePattern_CSPtr _oldLinePattern;
        const DxfLinePattern_CSPtr _newLinePattern;
    };
}