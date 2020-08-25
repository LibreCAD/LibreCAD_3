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
            /**
            * \brief Text Dialog
            */
            class TextDialog : public QDialog
            {
                Q_OBJECT

            public:
                /**
                * \brief Text Dialog Constructor
                * \param pointer to MainWindow
                */
                TextDialog(lc::ui::MainWindow* mainWindowIn, QWidget* parent = nullptr);
                
                /**
                * \brief Text Dialog Destructor
                */
                ~TextDialog();

            protected slots:
                /**
                * \brief Slot for the ok button being clicked
                */
                void okButtonClicked();

                /**
                * \brief Slot for the cancel button being clicked
                */
                void cancelButtonClicked();

                /**
                * \brief Slot for the insert button being clicked
                */
                void insertSymbolClicked();

            private:
                Ui::TextDialog* ui;
                QComboBox* fontComboBox;
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
