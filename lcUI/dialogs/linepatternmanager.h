#pragma once

#include <QDialog>
#include <QHeaderView>

#include <cad/document/document.h>

#include <dialogs/addlinepatterndialog.h>
#include <widgets/linepatternmodel.h>

namespace Ui {
    class LinePatternManager;
}

class LinePatternManager : public QDialog {
    Q_OBJECT

    public:
        LinePatternManager(lc::Document_SPtr document, QWidget* parent = 0);
        ~LinePatternManager();
        void setDocument(lc::Document_SPtr document);

    private slots:
        void on_editButton_pressed();
        void on_newButton_pressed();
        void on_lpView_doubleClicked(const QModelIndex& index);

    private:
        void updateModel();

        Ui::LinePatternManager* ui;
        lc::Document_SPtr _document;
        LinePatternModel* _model;

        void on_addLinePatternEvent(const lc::AddLinePatternEvent&);
        void on_removeLinePatternEvent(const lc::RemoveLinePatternEvent&);
        void on_replaceLinePatternEvent(const lc::ReplaceLinePatternEvent&);
};