#pragma once

#include <string>

namespace lc {
namespace storage {
namespace settings {
class ISettingValue {
public:
    virtual ~ISettingValue() {

    };

    /**
     * @brief Get setting value as double
     * @return setting value
     */
    virtual double getDouble() const = 0;

    /**
     * @brief Get setting value as float
     * @return setting value
     */
    virtual float getFloat() const = 0;

    /**
     * @brief Get setting value as int
     * @return setting value
     */
    virtual int getInt() const = 0;

    /**
     * @brief Get setting value as string
     * @return setting value
     */
    virtual std::string getString() const = 0;

    /**
     * @brief Set setting value
     * @param value
     * @throw SettingValueException
     */
    virtual void setValue(const std::string& value) = 0;

    /**
     * @brief Clone the setting
     * @return Setting clone
     */
    virtual ISettingValue* clone() const = 0;
};
}
}
}