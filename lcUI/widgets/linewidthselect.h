#pragma once

#include <QComboBox>
#include <QString>

#include <cad/meta/metalinewidth.h>
#include <widgets/linepatternpainter.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"

class LineWidthSelect : public QComboBox {
    public:
        LineWidthSelect(QWidget* parent = 0, bool showByLayer = false, bool showByBlock = false);
        lc::MetaLineWidthByValue_CSPtr lineWidth();

    private:
        void createEntries();

        QSize qIconSize;

        std::map<QString, double> values;
};