#pragma once

#include <QMenu>

#include "menuitem.h"

namespace lc
{
    namespace ui
    {
        namespace api
        {
            class Menu : public QMenu
            {
                Q_OBJECT

            public:
                Menu(const char* menuName, QWidget* parent = nullptr);

                Menu(QMenuBar* menuBar);

                Menu(QMenu* menu);

                void addItem(MenuItem* item);

                const std::string& getLabel();
            };
        }
    }
}
