#include <QDockWidget>
#include <cad/base/cadentity.h>
#include "widgets/guiAPI/inputguicontainer.h"
#include "widgets/guiAPI/inputgui.h"
#include "mainwindow.h"

namespace lc
{
    namespace ui
    {
        class PropertyEditor : public QDockWidget, public api::InputGUIContainer
        {
            Q_OBJECT

        public:
            static PropertyEditor* GetPropertyEditor(lc::ui::MainWindow* mainWindow = nullptr);

            void clear(std::vector<lc::entity::CADEntity_CSPtr> selectedEntities);

            void addEntity(lc::entity::CADEntity_CSPtr entity);

            bool addWidget(const std::string& key, api::InputGUI* guiWidget) override;

            bool addWidget(const std::string& key, api::ButtonGUI* buttonWidget) override;

            bool addWidget(const std::string& key, api::CheckBoxGUI* checkboxWidget) override;

            void propertyChanged(const std::string& key);

        private:
            PropertyEditor(lc::ui::MainWindow* mainWindow = nullptr);

        private:
            static PropertyEditor* instance;
            std::map<unsigned long, std::vector<std::string>> _selectedEntity;
            std::map<std::string, unsigned long> _widgetKeyToEntity;
        };
    }
}
