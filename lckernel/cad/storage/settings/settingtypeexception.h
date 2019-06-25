#pragma once

#include <exception>

namespace lc {
    namespace storage {
        namespace settings {
            class SettingTypeException : public std::runtime_error {
                public:
                    SettingTypeException(const std::string& __arg) : runtime_error(__arg) {

                    }
            };
        }
    }
}