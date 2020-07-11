#pragma once

#include "inputgui.h"
#include "mainwindow.h"

#include <vector>
#include <set>
#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

namespace Ui {
    class EntityGUI; 
}

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Entity Picker GUI Widget
            */
            class EntityGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief EntityGUI constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                EntityGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief EntityGUI destructor
                */
                ~EntityGUI();

                /**
                * \brief Add lua value to return table
                * \param LuaRef table
                */
                void getLuaValue(kaguya::LuaRef& table);

                /**
                * \brief Enable widget selection (called by dialog widget)
                * \param mainwindow pointer
                */
                void enableWidgetSelection(lc::ui::MainWindow* mainWindowIn);

                /**
                * \brief Add entity to the entity list
                * \param CADEntity shared pointer entity
                */
                void addEntity(lc::entity::CADEntity_CSPtr entity);

                /**
                * \brief Return entities
                * \return vector of cad entity pointers value
                */
                std::vector<lc::entity::CADEntity_CSPtr> value() const;

                /**
                * \brief Add entities to the list
                * \param vector of cadentity pointers
                */
                void setValue(std::vector<lc::entity::CADEntity_CSPtr> newSelectedEntities);

                /**
                * \brief Add callback for entity selected
                * \param LuaRef callback
                */
                void addCallback(kaguya::LuaRef cb);

            public slots:
                /**
                * \brief Called when entities are selected by the user in the cadmdichild
                */
                void entitySelected();

                /**
                * \brief Enable selections, called by addwidget button
                * \param bool toggle
                */
                void enableSelections(bool toggle);

                /**
                * \brief Run callbacks for item changed
                * \param QListWidgetItem pointer current item
                * \param QListWidgetItem pointer previous item
                */
                void itemChangedCallbacks(QListWidgetItem* current, QListWidgetItem* previous);

            private:
                QString getEntityInformation(lc::entity::CADEntity_CSPtr entity);

                void entityItemSelected(lc::entity::CADEntity_CSPtr entity);

            private:
                Ui::EntityGUI* ui;
                QLabel* _textLabel;
                QListWidget* _entityList;
                QPushButton* _addButton;

                bool _entitySelectionEnabled;
                lc::ui::MainWindow* mainWindow;

                std::vector<lc::entity::CADEntity_CSPtr> _selectedEntitiesList;
                std::set<unsigned int> _addedEntities;
                std::vector<kaguya::LuaRef> _callbacks;
            };
        }
    }
}
