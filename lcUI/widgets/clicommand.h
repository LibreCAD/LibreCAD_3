#pragma once

#include <QDockWidget>
#include <QResizeEvent>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>

#include <memory>
#include <cad/storage/settings.h>
#include <cad/geometry/geocoordinate.h>

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
                    bool addCommand(const std::string& name);

                    /**
                     * \brief Write a message in the logs
                     * \param message QString
                     */
                    void write(const QString& message);

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

                private:
                    bool checkParam(const QString& command);

                    void enterCommand(const QString& command);

                    void enterCoordinate(QString coordinate);

                    void enterNumber(double number);

                    Ui::CliCommand* ui;
                    std::shared_ptr<QCompleter> _completer;
                    std::shared_ptr<QStringListModel> _commands;
                    bool _returnText;

                    QStringList _history;
                    int _historySize;
                    int _historyIndex;
            };
        }
    }
}