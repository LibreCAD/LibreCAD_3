#pragma once

#include <mainwindow.h>
#include "inputgui.h"
#include "buttongui.h"
#include "checkboxgui.h"

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
                * \brief Add button directly (create a button group containing only that button)
                * \param pointer to ButtonGUI button
                * \return successfully added bool
                */
                virtual bool addWidget(const std::string& key, ButtonGUI* buttonWidget);

                /**
                * \brief Add checkbox directly (create a button group containing only that button)
                * \param pointer to CheckBoxGUI button
                * \return successfully added bool
                */
                virtual bool addWidget(const std::string& key, CheckBoxGUI* checkboxWidget);

                /**
                * \brief Return list of all input widgets
                * \return vector of pointer of InputGUI widgets
                */
                const std::vector<InputGUI*>& inputWidgets();

                /**
                * \brief Add lua callback for dialog finish
                * \return LuaRef callback
                */
                void addFinishCallback(kaguya::LuaRef cb);

                /**
                * \brief Generate table containing info of all widgets in dialog
                * \return LuaRef info table
                */
                kaguya::LuaRef generateDialogInfo(lua_State* luastate);

                /**
                * \brief Return list of all keys
                * \return vector of string keys
                */
                std::vector<std::string> keys() const;

                void removeInputGUI(const std::string& key);

            protected:
                std::map<std::string, InputGUI*> _inputWidgets;
                std::vector<kaguya::LuaRef> _callbacks;
                std::set<std::string> _addedKeys;
                lc::ui::MainWindow* mainWindow;
                std::string _label;
            };

        }
    }
}
