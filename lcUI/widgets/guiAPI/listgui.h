#pragma once

#include <QListWidget>
#include "inputgui.h"
#include <mainwindow.h>

#include <cad/builders/lwpolyline.h>

namespace Ui {
    class ListGUI;
}

namespace lc
{
    namespace ui
    {
        namespace api
        {
            class ListGUI : public InputGUI
            {
                Q_OBJECT

            public:
                enum class ListType
                {
                    NONE,
                    COORDINATE,
                    LW_VERTEX
                };

            public:
                ListGUI(std::string label, ListGUI::ListType listType = ListGUI::ListType::NONE, QWidget* parent = nullptr);

                ~ListGUI();

                void getLuaValue(kaguya::LuaRef& table) override;

                void addItem(const std::string& key, InputGUI* newitem);

                void setMainWindow(lc::ui::MainWindow* mainWindowIn);

                void setLabel(const std::string& newLabel) override;

                void setListType(ListGUI::ListType listType);

                void setListType(const std::string& listTypeStr);

                void addCallbackToAll(kaguya::LuaRef cb);

                void setValue(std::vector<lc::geo::Coordinate> coords);

                void setValue(std::vector<lc::builder::LWBuilderVertex> builderVertices);

                ListType listType() const;

                std::set<std::string> getKeys() const;

            public slots:
                void plusButtonClicked();

                void minusButtonClicked();

                void guiItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

            private:
                Ui::ListGUI* ui;
                QListWidget* listWidget;
                std::vector<InputGUI*> itemList;
                std::set<std::string> _addedKeys;
                lc::ui::MainWindow* mainWindow;
                ListType _listType;
                std::vector<kaguya::LuaRef> _callbacks;
                unsigned int itemIdCount;
                lc::entity::CADEntity_CSPtr _selectedCoordinate;
            };

        }
    }
}
