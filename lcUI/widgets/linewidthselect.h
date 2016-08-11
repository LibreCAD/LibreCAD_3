#pragma once

#include <QComboBox>
#include <QString>

#include <cad/meta/metalinewidth.h>
#include <widgets/linepatternpainter.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"

class LineWidthSelect : public QComboBox {
    Q_OBJECT

    public:
        LineWidthSelect(QWidget* parent = 0, bool showByLayer = false, bool showByBlock = false);
        lc::MetaLineWidthByValue_CSPtr lineWidth();
        void setWidth(double width);

    public slots:
        void onLayerChanged(lc::Layer_CSPtr layer);

    private:
        void createEntries();

        QSize qIconSize;

        std::map<QString, double> values;
};