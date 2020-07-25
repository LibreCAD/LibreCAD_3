#include <QDockWidget>
#include <QGroupBox>
#include <cad/base/cadentity.h>
#include "widgets/guiAPI/inputguicontainer.h"
#include "widgets/guiAPI/inputgui.h"
#include <QTreeWidget>
#include "mainwindow.h"

namespace lc
{
    namespace ui
    {
        /**
        * \brief Property Editor widget
        */
        class PropertyEditor : public QDockWidget, public api::InputGUIContainer
        {
            Q_OBJECT

        public:
            /**
            * \brief Return the property editor instance (create if does not exist)
            * \param pointer to MainWindow
            * \return pointer to PropertyEditor
            */
            static PropertyEditor* GetPropertyEditor(lc::ui::MainWindow* mainWindow = nullptr);

            /**
            * \brief Clear widgets and groups that were present before but not in the passed in new selected entities
            * \param vector of currently selected entities
            */
            void clear(std::vector<lc::entity::CADEntity_CSPtr> selectedEntities);

            /**
            * \brief Add property widgets for the entity
            * \param CADEntity shared pointer entity
            */
            void addEntity(lc::entity::CADEntity_CSPtr entity);

            /**
            * \brief Add input gui widget to property editor
            * \param string key
            * \param pointer to InputGUI gui widget
            */
            bool addWidget(const std::string& key, api::InputGUI* guiWidget) override;

            /**
            * \brief Add button gui widget to property editor
            * \param string key
            * \param pointer to ButtonGUI
            */
            bool addWidget(const std::string& key, api::ButtonGUI* buttonWidget) override;

            /**
            * \brief Add checkbox gui widget to property editor
            * \param string key
            * \param pointer to CheckBoxGUI
            */
            bool addWidget(const std::string& key, api::CheckBoxGUI* checkboxWidget) override;

            /**
            * \brief Function called by lua callbacks when a key is changed
            * \param string key of property widget changed
            */
            void propertyChanged(const std::string& key);

        private:
            /**
            * \brief Constructor of property editor (private because singleton)
            * \param pointer to MainWindow
            */
            PropertyEditor(lc::ui::MainWindow* mainWindow = nullptr);

            /**
            * \brief Helper function to create property widgets for the given entity properties map
            * \param unsigned long entity ID
            * \param PropertyMap entity properties
            */
            void createPropertiesWidgets(unsigned long entityID, const lc::entity::PropertiesMap& entityProperties);

            /**
            * \brief Helper function to create custom widgets for the entity (uses entity builder instead of entity properties here)
            * \param pointer to CADEntity
            */
            void createCustomWidgets(lc::entity::CADEntity_CSPtr entity);

            /**
            * \brief Helper function that deals with when a custom property widget value is changed
            */
            lc::entity::CADEntity_CSPtr customPropertyChanged(const std::string& key, const std::string& entityType, kaguya::LuaRef propertiesTable, lc::entity::CADEntity_CSPtr oldEntity);

            /**
            * \brief Helper function to generate the property key
            */
            std::string generatePropertyKey(unsigned long entityID, const std::string& propKey, int propType) const;

        private:
            static PropertyEditor* instance;
            std::map<unsigned long, std::vector<std::string>> _entityProperties;
            std::map<unsigned long, QTreeWidgetItem*> _entityGroup;
            std::set<unsigned long> _selectedEntities;
            std::map<std::string, unsigned long> _widgetKeyToEntity;
            unsigned long _currentEntity;
        };
    }
}
