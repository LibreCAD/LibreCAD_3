#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    namespace event {
        /**
         * \brief Event that gets emitted when a new line pattern was replaced in the document
         * \param DxfLinePatternByValue_CSPtr
         */
        class ReplaceLinePatternEvent {
            public:
                /**
                 * \brief Replace LinePattern Event
                 * \param LinePattern_CSPtr old line pattern
                 * \param LinePattern_CSPtr new line pattern
                 */
                ReplaceLinePatternEvent(const meta::DxfLinePatternByValue_CSPtr oldLinePattern,
                                        const meta::DxfLinePatternByValue_CSPtr newLinePattern)
                        : _oldLinePattern(oldLinePattern),
                          _newLinePattern(newLinePattern) {
                }

                /*!
                 * \brief Return LinePattern to replace
                 * \return DxfLinePatternByValue_CSPtr
                 */
                const meta::DxfLinePatternByValue_CSPtr oldLinePattern() const {
                    return _oldLinePattern;
                }

                /*!
                 * \brief Return LinePattern to add
                 * \return DxfLinePatternByValue_CSPtr
                 */
                const meta::DxfLinePatternByValue_CSPtr newLinePattern() const {
                    return _newLinePattern;
                }

            private:
                const meta::DxfLinePatternByValue_CSPtr _oldLinePattern;
                const meta::DxfLinePatternByValue_CSPtr _newLinePattern;
        };
    }
}