#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    /**
     * \brief Event that gets emited when a new line pattern was replaced in the document
     * \param DxfLinePatternByValue_CSPtr
     */
    class ReplaceLinePatternEvent {
    public:
        /**
         * \brief Replace LinePattern Event
         * \param LinePattern_CSPtr old line pattern
         * \param LinePattern_CSPtr new line pattern
         */
        ReplaceLinePatternEvent(const DxfLinePatternByValue_CSPtr oldLinePattern, const DxfLinePatternByValue_CSPtr newLinePattern)
                : _oldLinePattern(oldLinePattern), _newLinePattern(newLinePattern) {
        }

        /*!
         * \brief Return LinePattern to replace
         * \return DxfLinePatternByValue_CSPtr
         */
        const DxfLinePatternByValue_CSPtr oldLinePattern() const {
            return _oldLinePattern;
        }

        /*!
         * \brief Return LinePattern to add
         * \return DxfLinePatternByValue_CSPtr
         */
        const DxfLinePatternByValue_CSPtr newLinePattern() const {
            return _newLinePattern;
        }

    private:
        const DxfLinePatternByValue_CSPtr _oldLinePattern;
        const DxfLinePatternByValue_CSPtr _newLinePattern;
    };
}