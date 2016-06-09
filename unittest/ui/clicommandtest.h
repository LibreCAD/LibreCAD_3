#pragma once

#include <widgets/clicommand.h>

class CliCommandTest : public CliCommand {
	public:
		CliCommandTest() : CliCommand() {}

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