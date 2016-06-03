#pragma once

#include <QDockWidget>
#include <QResizeEvent>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>

#include <memory>

namespace Ui {
    class CliCommand;
}

class CliCommand : public QDockWidget {
        Q_OBJECT

    public:
        explicit CliCommand(QWidget* parent = 0);
        ~CliCommand();

		void keyPressEvent(QKeyEvent *event);

		bool addCommand(std::string name);

		void enterCommand(QString command);
		void write(QString message);
		void focus();

	public slots:
		void onReturnPressed();

	signals:
		void commandEntered(QString command);

    private:
		Ui::CliCommand* ui;
		std::shared_ptr<QCompleter> _completer;
		std::shared_ptr<QStringListModel> _commands;

		QStringList _history;
		int _historySize;
		int _historyIndex;
};

// CLICOMMAND_H
