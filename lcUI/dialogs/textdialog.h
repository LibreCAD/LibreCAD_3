#include <QDialog>

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
                TextDialog(QWidget* parent = nullptr);
                ~TextDialog();

            private:
                Ui::TextDialog* ui;
                QComboBox* drawingDirectionComboBox;
                QComboBox* halignComboBox;
                QComboBox* valignComboBox;
                QDoubleSpinBox* heightSpinBox;
                QDoubleSpinBox* angleSpinBox;
            };
        }
    }
}
