#pragma once

#include <string>
#include <stdexcept>
#include "settingtypeexception.h"
#include "isettingvalue.h"
#include <cad/logger/logger.h>

namespace lc {
namespace storage {
namespace settings {
class DoubleSettingValue : public ISettingValue {
private:
    double _value;

public:
    DoubleSettingValue();
    DoubleSettingValue(double value);

    double getDouble() const noexcept override;

    float getFloat() const noexcept override;

    int getInt() const noexcept override;

    std::string getString() const noexcept override;

    void setValue(const std::string& value) override;

    ISettingValue* clone() const override;
};
}
}
}