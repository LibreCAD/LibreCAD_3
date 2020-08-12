#include "copymanager.h"

#include <QApplication>
#include <QClipboard>

#include <iostream>

using namespace lc::ui;

CopyManager::CopyManager() {

}

void CopyManager::copyEntitiesToClipboard(const std::vector<lc::entity::CADEntity_CSPtr>& cadEntities) {
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setText(QString("Selected Entities - ") + QString::number(cadEntities.size()));
}

void CopyManager::pasteEvent() {
    QClipboard* clipBoard = QApplication::clipboard();
    std::cout << "Copied text - " << clipBoard->text().toStdString() << std::endl;
}
