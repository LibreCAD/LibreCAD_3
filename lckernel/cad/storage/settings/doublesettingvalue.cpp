#include "doublesettingvalue.h"
#include <iostream>

using namespace lc::storage::settings;

DoubleSettingValue::DoubleSettingValue() {
    _value = 0;
}

DoubleSettingValue::DoubleSettingValue(double value) {
    _value = value;
}

double DoubleSettingValue::getDouble() const noexcept {
    std::cout << _value << std::endl;
    return _value;
}

float DoubleSettingValue::getFloat() const noexcept {
    return (float) _value;
}

int DoubleSettingValue::getInt() const noexcept {
    return (int) _value;
}

std::string DoubleSettingValue::getString() const noexcept {
    return std::to_string(_value);
}

void DoubleSettingValue::setValue(const std::string& value) {
    try {
        _value = std::stod(value);
    }
    catch(...) {
        std::throw_with_nested(SettingTypeException("Unable to convert " + value + " to double."));
    }
}

ISettingValue* DoubleSettingValue::clone() const {
    return new DoubleSettingValue(_value);
}
