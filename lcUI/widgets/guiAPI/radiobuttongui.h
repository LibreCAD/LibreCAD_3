#pragma once

#include <QObject>
#include <QRadioButton>

// Phase 4 PR-5b — RadioButtonGUI does NOT inherit InputGUI, so we
// include the neutral callback + value headers directly.  kaguya
// finally leaves this header now that getValue(Map&) replaces
// getLuaValue(LuaRef&).
#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

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
    * \brief Add callback on state changed (phase 4 PR-5a: neutral)
    */
    void addCallback(lc::scripting::ScriptCallback cb);

    /**
    * \brief Write the checked-state into the neutral Map (PR-5b).
    * Not an override — RadioButtonGUI does not derive from InputGUI;
    * this method is called directly by HorizontalGroupGUI::getValue
    * and RadioGroupGUI::getValue.
    */
    void getValue(lc::scripting::Map& map);

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
    std::vector<lc::scripting::ScriptCallback> _callbacks;
    std::string _key;
};
}
}
}
