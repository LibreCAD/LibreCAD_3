#pragma once

#include <QTest>
#include <QThread>

#include <dialogs/addlinepatterndialog.cpp>

class AddLinePatternDialogTest : AddLinePatternDialog {
    public:
        AddLinePatternDialogTest(lc::storage::Document_SPtr document, QWidget* parent = 0) :
                AddLinePatternDialog(document, parent) {
        }

        AddLinePatternDialogTest(lc::storage::Document_SPtr document, lc::meta::DxfLinePatternByValue_CSPtr linePattern, QWidget* parent = 0) :
                AddLinePatternDialog(document, linePattern, parent) {
        }

        std::string name() {
            return ui->name->text().toStdString();
        }

        std::string description() {
            return ui->description->text().toStdString();
        }

        std::vector<double> path() {
            std::vector<double> path;
            int nbValues = _layout->count() - 1;

            for(int i = 0; i < nbValues; i++) {
                auto pathPart = static_cast<lc::ui::widgets::LinePatternPathPart *>(_layout->itemAt(i)->widget());

                if (!pathPart) {
                    continue;
                }

                switch (pathPart->type()) {
                    case lc::ui::widgets::LinePatternPathPart::PATH_SPACE:
                        path.push_back(-pathPart->value());
                        break;

                    case lc::ui::widgets::LinePatternPathPart::PATH_PLAIN:
                        path.push_back(pathPart->value());
                        break;

                    case lc::ui::widgets::LinePatternPathPart::PATH_DOT:
                        path.push_back(0);
                        break;
                }
            }

            return path;
        }

        void setName(std::string name) {
                ui->name->setText(name.c_str());
        }

        void setDescription(std::string description) {
                ui->description->setText(description.c_str());
        }

        void setPath(std::vector<double> path) {
            int nbValues = _layout->count() - 1;

            for(int i = 0; i < nbValues; i++) {
                auto item = _layout->takeAt(0);
                delete item->widget();
                delete item;
            }

            for(auto value : path) {
                QTest::mouseClick(ui->newValueButton, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5), 100);
                auto pathPart = _layout->itemAt(_layout->count() - 2)->widget();

                if(!pathPart) {
                    continue;
                }

                auto typeInput = pathPart->findChildren<QComboBox*>()[0];
                auto valueInput = pathPart->findChildren<QDoubleSpinBox*>()[0];

                if(value < 0) {
                    typeInput->setCurrentText(PATH_SPACE_NAME);
                    valueInput->setValue(-value);
                }
                else if(value == 0) {
                    typeInput->setCurrentText(PATH_DOT_NAME);
                    valueInput->setValue(0);
                }
                else {
                    typeInput->setCurrentText(PATH_PLAIN_NAME);
                    valueInput->setValue(value);
                }
            }
        }

        void create() {
            QTest::mouseClick(ui->saveButton, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5), 100);
        }

        void edit() {
            QTest::mouseClick(editButton, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5), 100);
        }
};