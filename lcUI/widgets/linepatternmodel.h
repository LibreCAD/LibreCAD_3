#pragma once

#include <QAbstractTableModel>
#include <QIcon>
#include <QPixmap>

#include <cad/meta/dxflinepattern.h>

#include <widgets/linepatternpainter.h>

#define PREVIEW_WIDTH 100
#define PREVIEW_HEIGHT 20

class LinePatternModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        enum columns {
            PREVIEW,
            NAME,
            DESCRIPTION,
            LAST
        };

        LinePatternModel(QObject* parent = 0);

        void setLinePatterns(std::vector<lc::DxfLinePattern_CSPtr> linePatterns);
        lc::DxfLinePattern_CSPtr linePatternAt(const int index) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    private:
        std::vector<lc::DxfLinePattern_CSPtr> _linePatterns;
};