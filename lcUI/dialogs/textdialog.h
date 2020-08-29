#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>

#include <mainwindow.h>
#include <cad/builders/text.h>

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

                /**
                * \brief Slot for alignment option being toggled
                */
                void alignmentToggled(bool toggle);

            private:
                /**
                * \brief Helper function to set alignment
                */
                void setAlignment(int i);

            private:
                Ui::TextDialog* ui;
                QComboBox* fontComboBox;
                QComboBox* drawingDirectionComboBox;
                QDoubleSpinBox* heightSpinBox;
                QDoubleSpinBox* angleSpinBox;
                QTextEdit* textEdit;
                lc::ui::MainWindow* _mainWindow;
                QGroupBox* alignmentGroupBox;

                lc::TextConst::HAlign halign;
                lc::TextConst::VAlign valign;
            };
        }
    }
}
