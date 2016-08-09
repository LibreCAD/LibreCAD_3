#pragma once

#include <QComboBox>

#include <cad/meta/dxflinepattern.h>
#include <widgets/linepatternpainter.h>
#include <dialogs/addlinepatterndialog.h>
#include <dialogs/linepatternmanager.h>
#include <cad/document/document.h>

#define BY_BLOCK "ByBlock"
#define BY_LAYER "ByLayer"
#define NEW_LP "New line pattern"
#define MANAGE_LP "Manage line patterns"

class LinePatternSelect : public QComboBox {
    Q_OBJECT

    public:
        LinePatternSelect(lc::Document_SPtr document = nullptr, QWidget *parent = 0, bool showByLayer = false, bool showByBlock = false);
        void setDocument(lc::Document_SPtr document = nullptr);

        lc::DxfLinePattern_CSPtr linePattern();

    private slots:
        void onActivated(const QString& text);

    private:
        void createEntries();

        QIcon generateQIcon(lc::DxfLinePattern_CSPtr linePattern);
        QSize qIconSize;

        bool _showByLayer;
        bool _showByBlock;

        lc::Document_SPtr _document;

        void on_addLinePatternEvent(const lc::AddLinePatternEvent&);
        void on_removeLinePatternEvent(const lc::RemoveLinePatternEvent&);
        void on_replaceLinePatternEvent(const lc::ReplaceLinePatternEvent&);
};