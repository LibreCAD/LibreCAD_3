#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QCheckBox>

#include <mainwindow.h>
#include <cad/primitive/mtext.h>
#include <cad/builders/mtext.h>

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

    /**
    * \brief Slot for text underline toggling
    */
    void underlineToggled(bool toggle);

    /**
    * \brief Slot for text strikethrough toggling
    */
    void strikethroughToggled(bool toggle);

    /**
    * \brief Slot for text bold toggling
    */
    void boldToggled(bool toggle);

    /**
    * \brief Slot for text italic toggling
    */
    void italicToggled(bool toggle);

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
    QCheckBox* underlineCheckBox;
    QCheckBox* strikethroughCheckBox;
    QCheckBox* boldCheckbox;
    QCheckBox* italicCheckbox;

    lc::TextConst::HAlign halign;
    lc::TextConst::VAlign valign;
};
}
}
}
