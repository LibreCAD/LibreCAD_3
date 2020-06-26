#pragma once

#include <QObject>
#include <QCheckBox>

#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief CheckBox GUI Widget
            */
            class CheckBoxGUI : public QCheckBox
            {
                Q_OBJECT

            public:
                /**
                * \brief CheckBox GUI Constructor
                * \param string button label
                * \param bool checked
                * \param parent qwidget parent
                */
                CheckBoxGUI(std::string label, bool checked=false, QWidget* parent = nullptr);

                /**
                * \brief Return label of the checkbox
                * \return string label
                */
                std::string label() const;

                /**
                * \brief Set label of checkbox
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

            public slots:
                /**
                * \brief Run all callbacks
                */
                void callbackCalled(int state);

            private:
                std::string _label;
                std::vector<kaguya::LuaRef> _callbacks;
                std::string _key;
            };
        }
    }
}
