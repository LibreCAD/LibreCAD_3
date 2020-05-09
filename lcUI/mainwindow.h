#pragma once

#include<QMainWindow>
#include "widgets/clicommand.h"
#include "Widgets/layers.h"
#include "widgets/linepatternselect.h"
#include "widgets/linewidthselect.h"
#include "widgets/colorselect.h"
#include "cadmdichild.h"

namespace lc
{
    namespace ui
    {
        namespace widgets
        {
            /*
                MainWindow GUI Initialization and Menu GUI API functions
            */
            class MainWindow : QMainWindow
            {
                Q_OBJECT
            public:
                /*
                    \ briefConstructor called during MainWindow
                    initialization during starting of program
                */
                MainWindow();
            private:
                CadMdiChild cadMdiChild;
                //QMainWindow* mainWindow;
                lc::ui::widgets::Layers layers;
                //lc::ui::widgets::CliCommand cliCommand;

                // Select tools
                lc::ui::widgets::LinePatternSelect linePatternSelect;
                lc::ui::widgets::LineWidthSelect lineWidthSelect;
                lc::ui::widgets::ColorSelect colorSelect;
            };
        }
    }
}
