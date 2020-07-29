#pragma once

#include <QObject>
#include <QPushButton>

#include "inputgui.h"
#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Button GUI Widget
            */
            class ButtonGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief Button GUI Constructor
                * \param string button label
                * \param parent qwidget parent
                */
                ButtonGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief Set label of button
                * \param string new label
                */
                void setLabel(const std::string& newLabel) override;

                /**
                * \brief Do nothing since button does not have a value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

                /**
                * \brief Add button callback
                * \param LuaRef lua callback
                */
                void addCallback(kaguya::LuaRef cb);

                /**
                * \brief Click the button
                */
                void click();

                /**
                * \brief Hide widget label
                */
                void hideLabel() override;

            public:
                QPushButton* _pushButton;

            public slots:
                /**
                * \brief Run all callbacks
                */
                void callbackCalled();

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
            };
        }
    }
}
