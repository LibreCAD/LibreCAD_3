#include "customizegrouptab.h"
#include <QTextStream>

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSpinBox>
#include <QLabel>

using namespace lc::ui::widgets;

CustomizeGroupTab::CustomizeGroupTab(QString groupName, QWidget *parent)
    :
      QWidget(parent),
      groupName(groupName)
{
    QTableView* tableView = new QTableView(this);
    dropModel = new OperationDropModel(groupName);
    tableView->setModel(dropModel);

    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setDragEnabled(true);
    tableView->setDropIndicatorShown(true);
    tableView->setAcceptDrops(true);

    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(tableView);

    QHBoxLayout* tableControlLayout = new QHBoxLayout();
    this->layout()->addItem(tableControlLayout);

    QLabel* rowControlLabel = new QLabel("Rows :- ", this);
    QSpinBox* rowControl = new QSpinBox(this);
    rowControl->setValue(dropModel->rowCount());

    QLabel* colControlLabel = new QLabel("Columns :- ", this);
    QSpinBox* colControl = new QSpinBox(this);
    colControl->setValue(dropModel->columnCount());

    rowControl->setMinimum(1);
    colControl->setMinimum(1);
    rowControl->setMaximum(5);
    colControl->setMaximum(5);

    connect(rowControl, QOverload<int>::of(&QSpinBox::valueChanged), dropModel, &OperationDropModel::setNumRows);
    connect(colControl, QOverload<int>::of(&QSpinBox::valueChanged), dropModel, &OperationDropModel::setNumCols);

    tableControlLayout->addWidget(rowControlLabel);
    tableControlLayout->addWidget(rowControl);
    tableControlLayout->addStretch();
    tableControlLayout->addWidget(colControlLabel);
    tableControlLayout->addWidget(colControl);
}

CustomizeGroupTab::CustomizeGroupTab(lc::ui::api::ToolbarGroup* toolbarGroup, QWidget* parent)
    :
    CustomizeGroupTab(toolbarGroup->label().c_str(), parent)
{
    std::vector<lc::ui::api::ToolbarButton*> buttonsList = toolbarGroup->buttons();

    int nCols = toolbarGroup->width();
    int nRows = buttonsList.size() / nCols;
    if (buttonsList.size() % nCols != 0) {
        nRows++;
    }

    dropModel->setNumCols(nCols);
    dropModel->setNumRows(nRows);
}
