#pragma once

#include <QWidget>

#include <cmath>

#define PATH_PLAIN_NAME "Plain"
#define PATH_SPACE_NAME "Space"
#define PATH_DOT_NAME "Dot"

namespace Ui {
    class LinePatternPathPart;
}

class LinePatternPathPart : public QWidget {
    Q_OBJECT

    public:
        enum PathType {
            PATH_PLAIN,
            PATH_SPACE,
            PATH_DOT
        };
        LinePatternPathPart(QWidget* parent = 0);
        LinePatternPathPart(double value, QWidget* parent = 0);

        PathType type();
        double value();

    signals:
        void update();

    private slots:
        void onTypeChanged(const QString& value);
        void onValueChanged(double value);

    private:
        Ui::LinePatternPathPart* ui;
};