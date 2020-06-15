#include "inputgui.h"
#include <QHBoxLayout>

#include "buttongui.h"

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Button Group GUI Widget
            */
            class ButtonGroupGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief Button Group GUI Constructor
                * \param string group label
                * \param parent qwidget parent
                */
                ButtonGroupGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief addd button to group
                * \param newButton pointer to ButtonGUI
                */
                void addButton(ButtonGUI* newButton);

            private:
                std::vector<ButtonGUI*> _buttons;
                QHBoxLayout* qhboxlayout;
            };
        }
    }
}
