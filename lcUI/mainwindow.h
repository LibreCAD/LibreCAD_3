#pragma once

#include "luainterface.h"
#include <kaguya/kaguya.hpp>

#include <QMainWindow>
#include "widgets/clicommand.h"
#include "Widgets/layers.h"
#include "widgets/linepatternselect.h"
#include "widgets/linewidthselect.h"
#include "widgets/colorselect.h"
#include "widgets/toolbar.h"
#include "cadmdichild.h"

namespace Ui {
    class MainWindow;
}

namespace lc
{
    namespace ui
    {
        /*
            MainWindow GUI Initialization and Menu GUI API functions
        */
        class MainWindow : public QMainWindow
        {
            Q_OBJECT
        public:
            /**
            * \brief Constructor for MainWindow
            */
            explicit MainWindow();

            ~MainWindow();

            /**
            * \brief Trigger appropriate signals for input events
            */
            void ConnectInputEvents();

            /**
            * \brief Gettters for clicommand,cadmdichild and toolbar for use in lua
            */
            lc::ui::widgets::CliCommand* getCliCommand();
            lc::ui::CadMdiChild* getCadMdiChild();
            lc::ui::widgets::Toolbar* getToolbar();

            /**
            * \brief Connect existing menu item to lua callback function
            * \param itemName item name , callback - function callback
            */
            void connectMenuItem(const std::string& itemName, kaguya::LuaRef callback);

            /**
            * \brief Run tool operation
            * \param operation Operation class , init_method - which init_method to run
            */
            void runOperation(kaguya::LuaRef operation, const std::string& init_method = "");

            /**
            * \brief Called on operation finish
            */
            void operationFinished();

            /**
            * \brief These will be called by runOperation for respective operation if extra icons
            *        are to be added during the operation
            * \param operation string to identify for which operation , options - list of functions to be run
            */
            void addOperationOptions(std::string operation, std::vector<kaguya::LuaRef> options);

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

            // Slot to deal with things to do in C++ after a point is triggered
            void triggerPoint(lc::geo::Coordinate coordinate);

            // File related slots
            void newFile();
            void openFile();

            // Edit slots
            void undo();
            void redo();
            void selectAll();
            void selectNone();
            void invertSelection();

        signals:
            void point(lc::geo::Coordinate coordinate);

        private:
            Ui::MainWindow* ui;
            lc::ui::LuaInterface luaInterface;

            lc::ui::CadMdiChild cadMdiChild;
            lc::ui::widgets::Layers layers;
            lc::ui::widgets::CliCommand cliCommand;
            lc::ui::widgets::Toolbar toolbar;

            // Select tools
            lc::ui::widgets::LinePatternSelect linePatternSelect;
            lc::ui::widgets::LineWidthSelect lineWidthSelect;
            lc::ui::widgets::ColorSelect colorSelect;

            lc::geo::Coordinate lastPoint;
            std::map<std::string, std::vector<kaguya::LuaRef>> operation_options;
        };
    }
}
