#pragma once

#pragma once

#include <string>
#include <stdexcept>
#include "settingtypeexception.h"
#include "isettingvalue.h"
#include <cad/logger/logger.h>

namespace lc {
namespace storage {
namespace settings {
class StringSettingValue : public ISettingValue {
private:
    std::string _value;

public:
    StringSettingValue() = default;
    StringSettingValue(std::string value);

    double getDouble() const override;

    float getFloat() const override;

    int getInt() const override;

    std::string getString() const noexcept override;

    void setValue(const std::string& value) override;

    ISettingValue* clone() const override;
};
}
}
}