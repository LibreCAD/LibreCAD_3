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
            class EntityGUI : public InputGUI
            {
                Q_OBJECT

            public:
                EntityGUI(std::string label, QWidget* parent = nullptr);

                ~EntityGUI();

                void getLuaValue(kaguya::LuaRef& table);

                void enableWidgetSelection(lc::ui::MainWindow* mainWindowIn);

                void addEntity(lc::entity::CADEntity_CSPtr entity);

                std::vector<lc::entity::CADEntity_CSPtr> value() const;

                void setValue(std::vector<lc::entity::CADEntity_CSPtr> newSelectedEntities);

            public slots:
                void entitySelected();

                void enableSelections(bool toggle);

            private:
                QString getEntityInformation(lc::entity::CADEntity_CSPtr entity);

            private:
                Ui::EntityGUI* ui;
                QLabel* _textLabel;
                QListWidget* _entityList;
                QPushButton* _addButton;

                bool _entitySelectionEnabled;
                lc::ui::MainWindow* mainWindow;

                std::vector<lc::entity::CADEntity_CSPtr> _selectedEntitiesList;
                std::set<unsigned int> _addedEntities;
            };
        }
    }
}
