#pragma once

#include <QObject>
#include <QPushButton>

#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Button GUI Widget
            */
            class ButtonGUI : public QPushButton
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
                * \brief Return label of the button
                * \return string label
                */
                std::string label() const;

                /**
                * \brief Set label of button
                * \param string new label
                */
                void setLabel(const std::string& newLabel);

                /**
                * \brief Add button callback
                * \param LuaRef lua callback
                */
                void addCallback(kaguya::LuaRef cb);

            public slots:
                /**
                * \brief Run all callbacks
                */
                void callbackCalled();

            private:
                std::string _label;
                std::vector<kaguya::LuaRef> _callbacks;
            };
        }
    }
}
