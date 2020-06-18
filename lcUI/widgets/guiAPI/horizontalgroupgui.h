#include "inputgui.h"
#include <QHBoxLayout>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Horizontal Group GUI Widget
            */
            class HorizontalGroupGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief Horizontal Group GUI Constructor
                * \param string group label
                * \param parent qwidget parent
                */
                HorizontalGroupGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief add button to group
                * \param newWidget pointer to QWidget
                */
                void addWidget(QWidget* newWidget);

                /**
                * \brief Return lua value
                * \param LuaRef value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

            private:
                std::vector<QWidget*> _widgets;
                QHBoxLayout* qhboxlayout;
            };
        }
    }
}
