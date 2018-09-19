#pragma once

#include <QWidget>

#include <cmath>

#define PATH_PLAIN_NAME "Plain"
#define PATH_SPACE_NAME "Space"
#define PATH_DOT_NAME "Dot"

namespace Ui {
    class LinePatternPathPart;
}

/**
 * \brief Model used to construct line pattern path
 */
class LinePatternPathPart : public QWidget {
    Q_OBJECT

    public:
        enum PathType {
            PATH_PLAIN,
            PATH_SPACE,
            PATH_DOT
        };
        /**
         * \brief Create empty model
         * \param parent Pointer to parent widget
         */
        LinePatternPathPart(QWidget* parent = nullptr);

        /**
         * \brief Create model pre-filled with value
         * \param value DXF size
         * \param parent Pointer to parent widget
         */
        LinePatternPathPart(double value, QWidget* parent = nullptr);

        /**
         * \brief Return type (plain, space or dot)
         * \return PATH_PLAIN, PATH_SPACE or PATH_DOT
         */
        PathType type();
        /**
         * \brief Return length
         * \return length (always positive)
         */
        double value();

    signals:
        void update();

    private slots:
        void onTypeChanged(const QString& value);
        void onValueChanged(double value);

    private:
        Ui::LinePatternPathPart* ui;
};