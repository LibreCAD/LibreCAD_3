#pragma once

#include <QDockWidget>
#include <QResizeEvent>

namespace Ui {
    class CliCommand;
}

class CliCommand : public QDockWidget {
        Q_OBJECT

    public:
        explicit CliCommand(QWidget* parent = 0);
        ~CliCommand();

    private:

    private:
        Ui::CliCommand* ui;
};

// CLICOMMAND_H
