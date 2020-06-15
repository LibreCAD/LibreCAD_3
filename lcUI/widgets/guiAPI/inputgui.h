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
            * \brief Input GUI Parent Widget
            */
            class InputGUI : public QWidget
            {
                Q_OBJECT

            public:
                /**
                * \brief InputGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                InputGUI(std::string _label, QWidget* parent = nullptr);

                /**
                * \brief Get GUI widget label
                * \return string gui label
                */
                std::string label() const;

                /**
                * \brief Set GUI widget label
                * \param string gui label
                */
                virtual void setLabel(const std::string& newLabel);

            private:
                std::string _label;
            };
        }
    }
}
