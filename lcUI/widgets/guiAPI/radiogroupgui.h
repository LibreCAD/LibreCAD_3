#include "inputgui.h"
#include <QButtonGroup>
#include <QHBoxLayout>

#include "radiobuttongui.h"

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Radio Group GUI Widget
            */
            class RadioGroupGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief Radio Group GUI Constructor
                * \param string group label
                * \param parent qwidget parent
                */
                RadioGroupGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief add radio button to group
                * \param newButton pointer to RadioButtonGUI
                */
                void addButton(RadioButtonGUI* radioButton);

                /**
                * \brief Return lua value
                * \param LuaRef value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

            private:
                QButtonGroup* qbuttongroup;
                QHBoxLayout* qhboxlayout;
            };
        }
    }
}
