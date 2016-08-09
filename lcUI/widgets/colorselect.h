#pragma once

#include <QComboBox>
#include <QString>
#include <QColor>

#include <cad/meta/metacolor.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"

class ColorSelect : public QComboBox {
public:
    ColorSelect(QWidget* parent = 0, bool showByLayer = false, bool showByBlock = false);
    lc::MetaColor_CSPtr color();

private:
    QSize qIconSize;
};