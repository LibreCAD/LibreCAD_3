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
    auto text = ui->command->text();
    bool isNumber;

    _history.push_front(text);

    if(_history.size() > _historySize) {
        _history.pop_back();
    }

    auto number = text.toDouble(&isNumber);
    if(isNumber) {
        enterNumber(number);
    }
    else if(text.indexOf(";") != -1 || text.indexOf(",") != -1) {
        enterCoordinate(text);
    }
    else {
        enterCommand(text);
    }

    ui->command->clear();
}

void CliCommand::keyPressEvent(QKeyEvent *event) {
    onKeyPressed(event);
}

void CliCommand::enterCommand(QString command) {
    if(command == "") {
        return;
    }

    auto completion = _completer->currentCompletion();

    if(command.compare(completion, Qt::CaseInsensitive) == 0) {
        write("Command: " + completion);
        emit commandEntered(completion);
    }
    else {
        write("Command " + command + " not found");
        ui->history->item(ui->history->count() - 1)->setForeground(Qt::red);
    }
}

void CliCommand::enterCoordinate(QString coordinate) {
    lc::geo::Coordinate point;
    QStringList numbers;

    if(coordinate.indexOf(";") != -1) {
        numbers = coordinate.split(";");
    }
    else {
        numbers = coordinate.split(",");
    }


    if(numbers.size() > 2) {
        point = lc::geo::Coordinate(numbers[0].toFloat(), numbers[1].toFloat(), numbers[2].toFloat());
    }
    else {
        point = lc::geo::Coordinate(numbers[0].toFloat(), numbers[1].toFloat());
    }

    auto message = QString("Coordinate: x=%1; y=%2; z=%3").arg(point.x()).arg(point.y()).arg(point.z());
    write(message);

    emit coordinateEntered(point);
}

void CliCommand::enterNumber(double number) {
    write(QString("Number: %1").arg(number));
    emit numberEntered(number);
}

void CliCommand::onKeyPressed(QKeyEvent *event) {
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

        default:
            ui->command->setFocus();
            ui->command->event(event);
            break;
    }
}