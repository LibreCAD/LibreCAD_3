#pragma once

#include <QPushButton>
#include <unordered_map>

#include <kaguya/kaguya.hpp>

namespace lc
{
    namespace ui
    {
        namespace api
        {
            /**
             * \brief GUI widget for toolbar buttons
             */
            class ToolbarButton : public QPushButton
            {
                Q_OBJECT

            public:
                /**
                * \brief ToolbarButton Constructor
                * \param string button label
                * \param string icon path
                * \param LuaRef callback
                */
                ToolbarButton(const char* buttonLabel, const char* icon, kaguya::LuaRef callback, const char* tooltip = "", bool _checkable=false, QWidget* parent = nullptr);

                /**
                * \brief ToolbarButton Constructor
                * \param string button label
                * \param string icon path
                */
                ToolbarButton(const char* buttonLabel, const char* icon, const char* tooltip = "", bool _checkable = false, QWidget* parent = nullptr);

                /**
                * \brief get label
                * \return string label
                */
                std::string label() const;

                /**
                * \brief Set group label
                * \param string new button name
                */
                void setLabel(const char* newLabel);

                /**
                * \brief Set button tool tip
                * \param string new tooltip
                */
                void setTooltip(const char* newToolTip);

                /**
                * \brief Add another lua callback
                * \param LuaRef callback
                */
                void addCallback(kaguya::LuaRef callback);

                /**
                * \brief Add another lua callback
                * \param LuaRef callback
                */
                void addCallback(const char* cb_name, kaguya::LuaRef callback);

                /**
                * \brief remove
                * \param LuaRef callback
                */
                void removeCallback(const char* cb_name);

                /**
                * \brief Change the button icon
                * \param string new icon path
                */
                void changeIcon(const char* newIconPath);

                /**
                * \brief Remove this button
                */
                void remove();

                /**
                * \brief Is button a checkable button
                */
                bool checkable() const;

                /**
                * \brief Get callback at index of callbacks list
                * \param int index
                * \return LuaRef& callback
                */
                kaguya::LuaRef& getCallback(int index);

                ToolbarButton* clone();

            signals:
                /**
                * \brief Signal to remove this button
                * \param ToolbarButton* pointer to this button
                */
                void removeButton(ToolbarButton*);

            public slots:

                void callbackCalled();

                void callbackCalledToggle(bool enabled);

            private:
                std::string _label;
                std::vector<kaguya::LuaRef> callbacks;
                std::unordered_map<std::string, int> namedCallbacks;
                bool _checkable;
            };
        }
    }
}
