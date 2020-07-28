#pragma once

#include <QObject>
#include <QCheckBox>

#include "inputgui.h"

#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief CheckBox GUI Widget
            */
            class CheckBoxGUI : public InputGUI
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
                void getLuaValue(kaguya::LuaRef& table) override;

                bool value() const;

                void setValue(bool check);

            public slots:
                /**
                * \brief Run all callbacks
                */
                void callbackCalled(int state);

            protected:
                /**
                * \brief Copy widget value to the clipboard
                */
                void copyValue(QDataStream& stream) override;

                /**
                * \brief Set widget value from clipboard
                */
                void pasteValue(QDataStream& stream) override;

            private:
                std::vector<kaguya::LuaRef> _callbacks;
                QCheckBox* _checkBox;
            };
        }
    }
}
