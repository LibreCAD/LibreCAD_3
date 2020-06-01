#pragma once

#include <QDockWidget>
#include <QResizeEvent>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>

#include <memory>
#include <QMap>
#include <cad/geometry/geocoordinate.h>

#include "widgettitlebar.h"

#include <kaguya/kaguya.hpp>

namespace Ui {
    class CliCommand;
}

namespace lc {
    namespace ui {
        namespace widgets {
            /**
             * \brief Command line widget
             */
            class CliCommand : public QDockWidget {
                Q_OBJECT

                public:
                    /**
                     * \brief Create widget
                     * \param parent Pointer to parent widget
                     */
                    explicit CliCommand(QWidget* parent = 0);

                    ~CliCommand();

                    void keyPressEvent(QKeyEvent* event);

                    /**
                     * \brief Add a new command
                     */
                    bool addCommand(const char* name, kaguya::LuaRef cb);

                    /**
                     * \brief Write a message in the logs
                     * \param message QString
                     */
                    void write(std::string message);

                    /**
                     * \brief Write text in input.
                     * \param text QString
                     * It's only used in unit tests for the moment. Maybe that can be moved to a new test class.
                     */
                    void setText(const QString& text);

                    /**
                     * \brief Return raw text to Lua.
                     * \param returnText true to return raw text, false to parse text
                     * Disables text parsing.
                     */
                    void returnText(bool returnText);

                    /**
                     * \brief Command is on or off.
                     * \param commandActive true when command is on, false when command is over
                     */
                    void commandActive(bool commandActive);

                    void runCommand(const char* command);

                    void enableCommand(const char* command, bool enable);

                    bool isCommandEnabled(const char* command) const;

                    std::vector<std::string> getAvailableCommands() const;

                    std::vector<std::string> getCommandsHistory() const;

                    void clear();

                public slots:

                    /**
                     * \brief Parse entered text.
                     * Check if the text is a number, a coordinate or a command and emit the right signal.
                     * This is a slot to allow getting key press events from other widgets.
                     */
                    void onReturnPressed();

                    /**
                     * \brief Process key events.
                     * Browse history if up or down key is pressed.
                     * This is a slot to allow getting key press events from other widgets.
                     */
                    void onKeyPressed(QKeyEvent* event);

                signals:

                    void commandEntered(QString command);

                    void coordinateEntered(lc::geo::Coordinate coordinate);

                    void relativeCoordinateEntered(lc::geo::Coordinate coordinate);

                    void numberEntered(double number);

                    void textEntered(QString text);

                    void finishOperation();

                private:
                    bool checkParam(const QString& command);

                    void enterCommand(const QString& command);

                    void enterCoordinate(QString coordinate);

                    void enterNumber(double number);

					void closeEvent(QCloseEvent* event);

                    Ui::CliCommand* ui;
                    std::shared_ptr<QCompleter> _completer;
                    std::shared_ptr<QStringListModel> _commands;
                    bool _returnText;
                    bool _commandActive;

                    QStringList _history;
                    int _historySize;
                    int _historyIndex;

                    QMap<QString, kaguya::LuaRef> _commands_cb;
                    QMap<QString, bool> _commands_enabled;
                    std::vector<std::string> _commands_entered;
            };
        }
    }
}
