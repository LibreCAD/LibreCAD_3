#include "linepatternmodel.h"

LinePatternModel::LinePatternModel(QObject *parent) :
    QAbstractTableModel(parent) {
}

void LinePatternModel::setLinePatterns(std::vector<lc::DxfLinePattern_CSPtr> linePatterns) {
    beginResetModel();

    _linePatterns = linePatterns;

    endResetModel();
}

lc::DxfLinePattern_CSPtr LinePatternModel::linePatternAt(const int index) const {
    return _linePatterns.at(index);
}

int LinePatternModel::rowCount(const QModelIndex&) const {
    return _linePatterns.size();
}

int LinePatternModel::columnCount(const QModelIndex&) const {
    return LAST;
}

QVariant LinePatternModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    auto linePattern = _linePatterns.at(index.row());

    if(role == Qt::DecorationRole && index.column() == PREVIEW) {
        QPixmap pixmap(PREVIEW_WIDTH, PREVIEW_HEIGHT);
        auto painter = LinePatternPainter(linePattern, &pixmap);

        painter.drawLinePattern();

        return pixmap;
    }

    if(role == Qt::SizeHintRole && index.column() == PREVIEW) {
        return QSize(PREVIEW_WIDTH, PREVIEW_HEIGHT);
    }

    if(role == Qt::DisplayRole) {
        switch (index.column()) {
            case NAME:
                return linePattern->name().c_str();

            case DESCRIPTION:
                return linePattern->description().c_str();
        }
    }

    return QVariant();
}

QVariant LinePatternModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            switch(section) {
                case PREVIEW:
                    return "Preview";

                case NAME:
                    return "Name";

                case DESCRIPTION:
                    return "Description";
            }
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}
