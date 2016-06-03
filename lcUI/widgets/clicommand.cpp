#include <QtWidgets/QtWidgets>
#include "clicommand.h"
#include "ui_clicommand.h"

CliCommand::CliCommand(QWidget* parent) :
    QDockWidget(parent),
    ui(new Ui::CliCommand),
    _historySize(10),
    _historyIndex(-1)
{
    ui->setupUi(this);

    connect(ui->command, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));

    _commands = std::make_shared<QStringListModel>();
    _completer = std::make_shared<QCompleter>();

    _completer->setCaseSensitivity(Qt::CaseInsensitive);
    _completer->setCompletionMode(QCompleter::InlineCompletion);
    _completer->setModel(_commands.get());

    ui->command->setCompleter(_completer.get());
}

CliCommand::~CliCommand() {
    delete ui;
}

bool CliCommand::addCommand(std::string name) {
    if(_commands->stringList().indexOf(name.c_str()) == -1) {
        auto newList = _commands->stringList();
        newList << QString(name.c_str());
        _commands->setStringList(newList);
        return true;
    }
    else {
        return false;
    }
}

void CliCommand::write(QString message) {
    ui->history->addItem(message);
    if(ui->history->count() > _historySize) {
        delete ui->history->takeItem(0);
    }
}

void CliCommand::onReturnPressed() {
    _history.push_front(ui->command->text());

    if(_history.size() > _historySize) {
        _history.pop_back();
    }

    enterCommand(ui->command->text());
}

void CliCommand::focus() {
    ui->command->setFocus();
}

void CliCommand::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_Up:
            if(_historyIndex + 1 < _history.size()) {
                _historyIndex++;
                ui->command->setText(_history[_historyIndex]);
            }
            break;
        case Qt::Key_Down:
            if(_historyIndex > 0) {
                _historyIndex--;
                ui->command->setText(_history[_historyIndex]);
            }
            else {
                _historyIndex = -1;
                ui->command->clear();
            }
            break;
    }
}

void CliCommand::enterCommand(QString command) {
    if(_commands->stringList().indexOf(command) == -1) {
        write("Command " + command + " not found");
        ui->history->item(ui->history->count() - 1)->setForeground(Qt::red);
    }
    else {
        write("Command: " + command);
        emit commandEntered(command);
    }

    ui->command->clear();
}