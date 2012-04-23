#include "clicommand.h"
#include "ui_clicommand.h"

CliCommand::CliCommand(QWidget* parent) :
    QDockWidget(parent),
    ui(new Ui::CliCommand) {
    ui->setupUi(this);
}

CliCommand::~CliCommand() {
    delete ui;
}

