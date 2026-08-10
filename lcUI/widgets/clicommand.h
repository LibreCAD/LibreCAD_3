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

// Phase 4 PR-2 — CliCommand refactored to store neutral ScriptCallbacks
// instead of raw kaguya::LuaRef.  guibridge.cpp wraps the Lua-side call
// site (which still receives a LuaRef from Lua) via
// lc::lua::makeLuaCallback().  Kaguya include no longer required here.
#include <lcscripting/scriptcallback.h>

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
     * \param name  command name (uppercase by convention)
     * \param cb    callback to invoke when the command is run
     *
     * The callback is stored as a language-neutral `ScriptCallback`
     * (phase 4 PR-2 refactor).  Lua-side call sites are wrapped in
     * guibridge.cpp via `lc::lua::makeLuaCallback(luaRef)`.
     */
    bool addCommand(const char* name, lc::scripting::ScriptCallback cb);

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

    /**
     * \brief run the command
     * \param command string
     */
    void runCommand(const char* command);

    /**
     * \brief Enable/Disable the command
     * \param command string
     * \param enable bool
     */
    void enableCommand(const char* command, bool enable);

    /**
     * \brief Is the command enabled
     * \param command string
     * \return enable bool
     */
    bool isCommandEnabled(const char* command) const;

    /**
     * \brief Get all available commands
     * \return vector of command strings
     */
    std::vector<std::string> availableCommands() const;

    /**
     * \brief Get the history of commands run
     * \return vector of command strings
     */
    std::vector<std::string> commandsHistory() const;

    /**
     * \brief Clear the clicommand output in the widget
     */
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

    QMap<QString, lc::scripting::ScriptCallback> _commands_cb;
    QMap<QString, bool> _commands_enabled;
    std::vector<std::string> _commands_entered;
};
}
}
}
