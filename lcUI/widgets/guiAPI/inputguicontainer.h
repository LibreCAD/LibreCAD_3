#pragma once

#include <mainwindow.h>
#include "inputgui.h"
#include "buttongui.h"
#include "checkboxgui.h"

#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

namespace lc
{
namespace ui
{
namespace api
{
class InputGUIContainer
{
public:
    /**
    * \brief InputGUIContainer Constructor
    * \param string label
    * \param pointer to mainwindow
    */
    InputGUIContainer(const std::string& label, lc::ui::MainWindow* mainWindow);

    /**
    * \brief InputGUIContainer Destructor
    */
    ~InputGUIContainer();

    /**
    * \brief Set InputGUIContainer label
    */
    void setLabel(const std::string& newlabel);

    /**
    * \brief Add input gui widget
    * \param pointer to InputGUI widget
    * \return successfully added bool
    */
    virtual bool addWidget(const std::string& key, InputGUI* guiWidget);

    /**
    * \brief Return list of all input widgets
    * \return vector of pointer of InputGUI widgets
    */
    std::vector<InputGUI*> inputWidgets();

    /**
    * \brief Add finish callback (phase 4 PR-5b: neutral ScriptCallback).
    */
    void addFinishCallback(lc::scripting::ScriptCallback cb);

    /**
    * \brief Build a language-neutral Map of every child widget's current
    * value.  DialogWidget::finishCallbacks and PropertyEditor::propertyChanged
    * both consume this — adapters materialize it per runtime when the Map
    * flows through ScriptCallback::call.
    */
    lc::scripting::Map generateInfo();

    /**
    * \brief Return list of all keys
    * \return vector of string keys
    */
    std::vector<std::string> keys() const;

    /**
    * \brief Remove input gui from the input gui container
    * \param call delete on the input gui widget
    * \return vector of string keys
    */
    void removeInputGUI(std::string key, bool deleteWidget = true);

protected:
    std::map<std::string, InputGUI*> _inputWidgets;
    std::map<std::string, std::string> _widgetToGroup;
    std::vector<lc::scripting::ScriptCallback> _callbacks;
    std::set<std::string> _addedKeys;
    lc::ui::MainWindow* mainWindow;
    std::string _label;
};

}
}
}
