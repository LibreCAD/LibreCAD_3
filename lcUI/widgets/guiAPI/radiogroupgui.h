#include "inputgui.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <set>

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
                void addButton(const std::string& key, RadioButtonGUI* radioButton);

                /**
                * \brief Return lua value
                * \param LuaRef value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

                /**
                * \brief Get keys of all widgets added to the group
                * \param set of string keys
                */
                std::set<std::string> getKeys();

                /**
                * \brief Hide widget label
                */
                void hideLabel() override;

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
                QButtonGroup* qbuttongroup;
                QHBoxLayout* qhboxlayout;
                std::set<std::string> _addedKeys;
            };
        }
    }
}
