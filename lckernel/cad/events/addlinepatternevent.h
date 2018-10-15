#pragma once

#include <cad/meta/dxflinepattern.h>

namespace lc {
    namespace event {
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
                AddLinePatternEvent(const meta::DxfLinePatternByValue_CSPtr linePattern) : _linePattern(linePattern) {
                }

                /*!
                 * \brief Returns the line pattern.
                 * \return DxfLinePatternByValue_CSPtr linePattern.
                 */
                const meta::DxfLinePatternByValue_CSPtr linePattern() const {
                    return _linePattern;
                }

            private:
                const meta::DxfLinePatternByValue_CSPtr _linePattern;
        };
    }
}
