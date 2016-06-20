#pragma once

#include <QDockWidget>
#include <QResizeEvent>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>

#include <memory>

#include <cad/geometry/geocoordinate.h>

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

		void write(QString message);
		void setText(QString text);

		void returnText(bool returnText);

	public slots:
		void onReturnPressed();
		void onKeyPressed(QKeyEvent *event);

	signals:
		void commandEntered(QString command);
		void coordinateEntered(lc::geo::Coordinate coordinate);
		void relativeCoordinateEntered(lc::geo::Coordinate coordinate);
		void numberEntered(double number);
		void textEntered(QString text);

    private:
		void enterCommand(QString command);
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

// CLICOMMAND_H
