#pragma once

#include <widgets/clicommand.h>
#include "cad/geometry/geocoordinate.h"
#include <QObject>
#include <QMetaObject>

using namespace lc::ui::widgets;

Q_DECLARE_METATYPE(lc::geo::Coordinate);

class CliCommandTest : public CliCommand {
public:
    CliCommandTest() : CliCommand() {
        qRegisterMetaType<lc::geo::Coordinate>();
    }

    /* @brief testCommand
     * simulate a command
     * @param QString
     */
    void testCommand(QString command) {
        auto enterEvent = QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);

        setText(command);
        onKeyPressed(&enterEvent);
    }
};
