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

                /**
                * \brief add key and value to the lua table
                * \param LuaRef lua table
                */
                virtual void getLuaValue(kaguya::LuaRef& table) = 0;

                /**
                * \brief Set gui key for the lua table
                * \param string gui key
                */
                virtual void setKey(const std::string& keyIn);

                const std::string key() const;

            protected:
                std::string _label;
                std::string _key;
            };
        }
    }
}
