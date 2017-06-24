#pragma once

#include <QAbstractTableModel>
#include <QIcon>
#include <QPixmap>

#include <cad/meta/dxflinepattern.h>

#include <widgets/linepatternpainter.h>

#define PREVIEW_WIDTH 100
#define PREVIEW_HEIGHT 20

/**
 * \brief Model to display a list of DxfLinePatterns in a QTableView.
 * Show preview, name and description.
 */
class LinePatternModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        enum columns {
            PREVIEW,
            NAME,
            DESCRIPTION,
            LAST
        };

        /**
         * \brief Create widget
         * \param parent Pointer to parent object
         */
        LinePatternModel(QObject* parent = 0);

        /**
         * \brief Set the new list of line patterns.
         * \param linePatterns New list of DxfLinePatterns
         * Update the list
         */
        void setLinePatterns(std::vector<lc::DxfLinePatternByValue_CSPtr> linePatterns);

        /**
         * \brief Return line pattern at specific row.
         * \param index Row
         * \return Shared pointer to DxfLinePattern
         */
        lc::DxfLinePattern_CSPtr linePatternAt(const int index) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    private:
        std::vector<lc::DxfLinePatternByValue_CSPtr> _linePatterns;
};