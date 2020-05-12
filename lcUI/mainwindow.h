#pragma once

#include "luainterface.h"
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
        class LuaInterface;
        /*
            MainWindow GUI Initialization and Menu GUI API functions
        */
        class MainWindow : public QMainWindow
        {
            Q_OBJECT
        public:
            /**
            * \brief Constructor for MainWindow
            * \param luaInterface pointer to luaInterface instance
            */
            MainWindow(lc::ui::LuaInterface* luaInterface);

            /*
             * \brief Create menu
             */
            QAction* createMenu();

            /**
            * \brief Trigger appropriate signals for input events
            */
            void ConnectInputEvents();

            lc::ui::widgets::CliCommand* getCliCommand();
            lc::ui::CadMdiChild* getCadMdiChild();

        public slots:
            // CadMdiChild slots
            void triggerMousePressed();
            void triggerMouseReleased();
            void triggerMouseMoved();
            void triggerKeyPressed(int key);

            // CliCommand slots
            void triggerCoordinateEntered(lc::geo::Coordinate coordinate);
            void triggerRelativeCoordinateEntered(lc::geo::Coordinate coordinate);
            void triggerNumberEntered(double number);
            void triggerTextEntered(QString text);
            void triggerFinishOperation();
            void triggerCommandEntered(QString command);

        private:
            lc::ui::CadMdiChild cadMdiChild;
            lc::ui::widgets::Layers layers;
            lc::ui::widgets::CliCommand cliCommand;
            lc::ui::widgets::Toolbar toolbar;

            // Select tools
            lc::ui::widgets::LinePatternSelect linePatternSelect;
            lc::ui::widgets::LineWidthSelect lineWidthSelect;
            lc::ui::widgets::ColorSelect colorSelect;

            lc::ui::LuaInterface* luaInterface;
            lc::geo::Coordinate lastPoint;
        };
    }
}
