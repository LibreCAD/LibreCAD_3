#pragma once

#include <QComboBox>

#include <cad/meta/dxflinepattern.h>
#include <widgets/linepatternpainter.h>
#include <dialogs/addlinepatterndialog.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"

class LinePatternSelect : public QComboBox {
    public:
        LinePatternSelect(std::vector<lc::DxfLinePattern_CSPtr>& linePatterns, QWidget *parent);
        void addByBlock();
        void addByLayer();

        lc::DxfLinePattern_CSPtr linePattern();

    private slots:
        void onCurrentIndexChanged(const QString& text);

    private:
        QIcon generateQIcon(lc::DxfLinePattern_CSPtr linePattern);
        QSize qIconSize;

        std::vector<lc::DxfLinePattern_CSPtr> _linePatterns;
};