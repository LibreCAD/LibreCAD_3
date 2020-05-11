#pragma once

#include<QMainWindow>
#include "widgets/clicommand.h"
#include "Widgets/layers.h"
#include "widgets/linepatternselect.h"
#include "widgets/linewidthselect.h"
#include "widgets/colorselect.h"
#include "widgets/toolbar.h"
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
                 *  \ brief  Constructor called during MainWindow
                 */
                MainWindow();

                /*
                 * \brief Create menu
                 */
                QAction* createMenu();

                lc::ui::widgets::CliCommand* getCliCommand();
                lc::ui::CadMdiChild* getCadMdiChild();

            private:
                lc::ui::CadMdiChild cadMdiChild;
                lc::ui::widgets::Layers layers;
                lc::ui::widgets::CliCommand cliCommand;
                lc::ui::widgets::Toolbar toolbar;

                // Select tools
                lc::ui::widgets::LinePatternSelect linePatternSelect;
                lc::ui::widgets::LineWidthSelect lineWidthSelect;
                lc::ui::widgets::ColorSelect colorSelect;
            };
        }
    }
}
