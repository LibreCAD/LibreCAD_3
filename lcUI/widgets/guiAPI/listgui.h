#pragma once

#include <QListWidget>
#include "inputgui.h"
#include <mainwindow.h>

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
                ListGUI(std::string label, QWidget* parent = nullptr);

                ~ListGUI();

                void getLuaValue(kaguya::LuaRef& table) override;

                void addItem(const std::string& key, InputGUI* newitem);

                void setMainWindow(lc::ui::MainWindow* mainWindowIn);

                void setLabel(const std::string& newLabel) override;

                std::set<std::string> getKeys() const;

            private:
                Ui::ListGUI* ui;
                QListWidget* listWidget;
                std::vector<InputGUI*> itemList;
                std::set<std::string> _addedKeys;
                lc::ui::MainWindow* mainWindow;
            };

        }
    }
}
