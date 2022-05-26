#include "stringsettingvalue.h"

using namespace lc::storage::settings;

StringSettingValue::StringSettingValue(std::string value) {
    _value = std::move(value);
}

double StringSettingValue::getDouble() const {
    throw SettingTypeException("String can't be converted to double");
}

float StringSettingValue::getFloat() const {
    throw SettingTypeException("String can't be converted to float");
}

int StringSettingValue::getInt() const {
    throw SettingTypeException("String can't be converted to int");
}

std::string StringSettingValue::getString() const noexcept {
    return _value;
}

void StringSettingValue::setValue(const std::string& value) {
    _value = value;
}

ISettingValue* StringSettingValue::clone() const {
    return new StringSettingValue(_value);
}
