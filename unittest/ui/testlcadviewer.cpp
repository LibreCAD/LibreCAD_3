#include <gtest/gtest.h>

#include <QString>

#include "lcadviewer.h"

TEST(LcadViewerGlInitialization, FailureMessageIncludesCauseAndWaylandHint) {
    const QString message = lc::ui::LCADViewer::glInitializationFailureMessage(
        QStringLiteral("GLEW Error: Unknown error"));

    EXPECT_TRUE(message.contains(QStringLiteral("GLEW Error: Unknown error")));
    EXPECT_TRUE(message.contains(QStringLiteral("QT_QPA_PLATFORM=xcb")));
}

TEST(LcadViewerGlInitialization, FailureMessageWithoutDetailStillExplainsFailure) {
    const QString message = lc::ui::LCADViewer::glInitializationFailureMessage(QString());

    EXPECT_FALSE(message.trimmed().isEmpty());
    EXPECT_TRUE(message.contains(QStringLiteral("QT_QPA_PLATFORM=xcb")));
}
