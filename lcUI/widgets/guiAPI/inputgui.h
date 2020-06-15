#pragma once

#include <QObject>
#include <QMetaObject>
#include <QWidget>

#include <kaguya/kaguya.hpp>

#include <string>
#include <vector>

namespace lc
{
    namespace ui
    {
        namespace api
        {
            /**
            * \brief GUI widget for menu
            */
            class InputGUI : public QWidget
            {
                Q_OBJECT

            public:
                InputGUI(std::string _label, QWidget* parent = nullptr);

                std::string label() const;

                virtual void setLabel(const std::string& newLabel);

            private:
                std::string _label;
            };
        }
    }
}
