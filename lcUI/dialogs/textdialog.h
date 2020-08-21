#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>

#include <mainwindow.h>

namespace Ui {
    class TextDialog;
}

namespace lc {
    namespace ui {
        namespace dialog {
            class TextDialog : public QDialog
            {
                Q_OBJECT

            public:
                TextDialog(lc::ui::MainWindow* mainWindowIn, QWidget* parent = nullptr);
                ~TextDialog();

            protected slots:
                void okButtonClicked();

                void cancelButtonClicked();

            private:
                Ui::TextDialog* ui;
                QComboBox* drawingDirectionComboBox;
                QComboBox* halignComboBox;
                QComboBox* valignComboBox;
                QDoubleSpinBox* heightSpinBox;
                QDoubleSpinBox* angleSpinBox;
                QTextEdit* textEdit;
                lc::ui::MainWindow* _mainWindow;
            };
        }
    }
}
