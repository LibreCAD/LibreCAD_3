#pragma once

#include <QObject>
#include <QRadioButton>

#include <kaguya/kaguya.hpp>

namespace lc {
namespace ui {
namespace api {
/**
* \brief CheckBox GUI Widget
*/
class RadioButtonGUI : public QRadioButton
{
    Q_OBJECT

public:
    /**
    * \brief RadioButton GUI Constructor
    * \param string button label
    * \param bool checked
    * \param parent qwidget parent
    */
    RadioButtonGUI(std::string label, QWidget* parent = nullptr);

    /**
    * \brief Return label of the radiobutton
    * \return string label
    */
    std::string label() const;

    /**
    * \brief Set label of radiobutton
    * \param string new label
    */
    void setLabel(const std::string& newLabel);

    /**
    * \brief Add callback on state changed
    * \param LuaRef lua callback
    */
    void addCallback(kaguya::LuaRef cb);

    /**
    * \brief Return lua value
    * \param LuaRef value
    */
    void getLuaValue(kaguya::LuaRef& table);

    /**
    * \brief Set gui key for the lua table
    * \param string gui key
    */
    void setKey(const std::string& keyIn);

    /**
    * \brief Return whether radio button is checked
    * \return bool value
    */
    bool value() const;

    /**
    * \brief Set radio button toggle
    * \param bool toggle
    */
    void setValue(bool toggle);

public slots:
    /**
    * \brief Run all callbacks
    */
    void callbackCalled(bool toggled);

private:
    std::string _label;
    std::vector<kaguya::LuaRef> _callbacks;
    std::string _key;
};
}
}
}
