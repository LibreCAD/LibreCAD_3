#pragma once

#include <QComboBox>
#include <QString>
#include <QColor>
#include <QColorDialog>

#include <cad/meta/metacolor.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"
#define CUSTOM "Custom"

class ColorSelect : public QComboBox {
    Q_OBJECT

    public:
        ColorSelect(QWidget* parent = 0, bool showByLayer = false, bool showByBlock = false);
        lc::MetaColor_CSPtr metaColor();
        lc::Color color();

    private slots:
        void onActivated(const QString& text);
        void on_customColorChanged(const QColor& color);

    private:
        QColor getColor();

        QSize qIconSize;
        QColor _customColor;
};