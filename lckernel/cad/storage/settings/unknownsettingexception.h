#pragma once

#include <stdexcept>

namespace lc {
    namespace storage {
        namespace settings {
            class UnknownSettingException : public std::runtime_error {
                public:
                    UnknownSettingException(const std::string& __arg) : runtime_error(__arg) {

                    }
            };
        }
    }
}