#include <QApplication>
#include <QSignalSpy>

#include <gtest/gtest.h>
#include "clicommandtest.h"

#include <kaguya/kaguya.hpp>

#include "uitests.h"

TEST(CommandLine, CommandTest) {
    QApplication app(argc, argv);
    CliCommandTest cliTest;

    QSignalSpy commandSpy(&cliTest, SIGNAL(commandEntered(QString)));
    EXPECT_TRUE(commandSpy.isValid());

    QString c1 = "COMMAND";
    QString c2 = "INVALID";

    cliTest.addCommand(c1.toStdString().c_str(), kaguya::LuaRef());
    cliTest.testCommand(c1);
    cliTest.testCommand(c2);

    EXPECT_EQ(1, commandSpy.count());

    auto commandReceived = commandSpy.takeFirst().at(0).toString();

    EXPECT_EQ(c1, commandReceived);
}

TEST(CommandLine, CoordinateTest) {
    QApplication app(argc, argv);
    CliCommandTest cliTest;

    qRegisterMetaType<lc::geo::Coordinate>();
    QSignalSpy coordinateSpy(&cliTest, SIGNAL(coordinateEntered(lc::geo::Coordinate)));
    EXPECT_TRUE(coordinateSpy.isValid());

    lc::geo::Coordinate c1(1.1, 1.2);
    QString c1_Str = "1.1,1.2";

    lc::geo::Coordinate c2(1.1, 1.2, 1.3);
    QString c2_Str = "1.1;1.2;1.3";

    cliTest.testCommand(c1_Str);
    cliTest.testCommand(c2_Str);

    EXPECT_EQ(2, coordinateSpy.count());

    auto c1_Received = qvariant_cast<lc::geo::Coordinate>(coordinateSpy.at(0).at(0));
    auto c2_Received = qvariant_cast<lc::geo::Coordinate>(coordinateSpy.at(1).at(0));

    EXPECT_FLOAT_EQ(c1.x(), c1_Received.x());
    EXPECT_FLOAT_EQ(c1.y(), c1_Received.y());
    EXPECT_FLOAT_EQ(c1.z(), c1_Received.z());

    EXPECT_FLOAT_EQ(c2.x(), c2_Received.x());
    EXPECT_FLOAT_EQ(c2.y(), c2_Received.y());
    EXPECT_FLOAT_EQ(c2.z(), c2_Received.z());
}

TEST(CommandLine, RelativeCoordinateTest) {
    QApplication app(argc, argv);
    CliCommandTest cliTest;

    qRegisterMetaType<lc::geo::Coordinate>();
    QSignalSpy coordinateSpy(&cliTest, SIGNAL(relativeCoordinateEntered(lc::geo::Coordinate)));
    EXPECT_TRUE(coordinateSpy.isValid());

    lc::geo::Coordinate c1(1.1, 1.2);
    QString c1_Str = "@1.1,1.2";

    lc::geo::Coordinate c2(1.1, 1.2, 1.3);
    QString c2_Str = "@1.1;1.2;1.3";

    cliTest.testCommand(c1_Str);
    cliTest.testCommand(c2_Str);

    EXPECT_EQ(2, coordinateSpy.count());

    auto c1_Received = qvariant_cast<lc::geo::Coordinate>(coordinateSpy.at(0).at(0));
    auto c2_Received = qvariant_cast<lc::geo::Coordinate>(coordinateSpy.at(1).at(0));

    EXPECT_FLOAT_EQ(c1.x(), c1_Received.x());
    EXPECT_FLOAT_EQ(c1.y(), c1_Received.y());
    EXPECT_FLOAT_EQ(c1.z(), c1_Received.z());

    EXPECT_FLOAT_EQ(c2.x(), c2_Received.x());
    EXPECT_FLOAT_EQ(c2.y(), c2_Received.y());
    EXPECT_FLOAT_EQ(c2.z(), c2_Received.z());
}

TEST(CommandLine, NumberTest) {
    QApplication app(argc, argv);
    CliCommandTest cliTest;

    qRegisterMetaType<lc::geo::Coordinate>();
    QSignalSpy numberSpy(&cliTest, SIGNAL(numberEntered(double)));
    EXPECT_TRUE(numberSpy.isValid());

    double n1 = 1;
    QString n1_Str = "1";

    double n2 = 1.234;
    QString n2_Str = "1.234";

    cliTest.testCommand(n1_Str);
    cliTest.testCommand(n2_Str);

    EXPECT_EQ(2, numberSpy.count());

    auto n1_Received = numberSpy.at(0).at(0).toDouble();
    auto n2_Received = numberSpy.at(1).at(0).toDouble();

    EXPECT_FLOAT_EQ(n1, n1_Received);
    EXPECT_FLOAT_EQ(n2, n2_Received);
}

TEST(CommandLine, EnableCommandTest) {
    QApplication app(argc, argv);

    kaguya::State state;
    CliCommandTest cliTest;
    state.dostring("testvalue = false");
    state.dostring("testfunction = function() testvalue=true end");

    const char *c1 = "COMMAND";
    cliTest.addCommand(c1, state["testfunction"]);

    EXPECT_TRUE(cliTest.isCommandEnabled("COMMAND"));
    EXPECT_FALSE(state["testvalue"].get<bool>());

    cliTest.enableCommand(c1, false);
    cliTest.runCommand(c1);

    EXPECT_FALSE(cliTest.isCommandEnabled("COMMAND"));
    EXPECT_FALSE(state["testvalue"].get<bool>());

    cliTest.enableCommand(c1, true);
    cliTest.runCommand(c1);

    EXPECT_TRUE(cliTest.isCommandEnabled("COMMAND"));
    EXPECT_TRUE(state["testvalue"].get<bool>());
}

TEST(CommandLine, GetAPITest) {
    QApplication app(argc, argv);
    kaguya::State state;
    CliCommandTest cliTest;

    std::vector<std::string> availableCommands = cliTest.availableCommands();
    std::vector<std::string> commandsEntered = cliTest.commandsHistory();

    state.dostring("testfunction = function() testvalue=true end");

    EXPECT_EQ(0, availableCommands.size());
    EXPECT_EQ(0, commandsEntered.size());

    cliTest.addCommand("COMMAND1", state["testfunction"]);
    cliTest.addCommand("COMMAND2", state["testfunction"]);

    availableCommands = cliTest.availableCommands();
    commandsEntered = cliTest.commandsHistory();

    EXPECT_EQ(2, availableCommands.size());
    EXPECT_EQ(0, commandsEntered.size());

    cliTest.runCommand("COMMAND1");
    cliTest.enableCommand("COMMAND2", false);
    cliTest.runCommand("COMMAND2");

    availableCommands = cliTest.availableCommands();
    commandsEntered = cliTest.commandsHistory();

    EXPECT_EQ(2, availableCommands.size());
    EXPECT_EQ(1, commandsEntered.size());
}
