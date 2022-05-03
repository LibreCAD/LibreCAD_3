#pragma once

#include "inputgui.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>

namespace Ui {
class ComboBoxGUI;
}

namespace lc
{
namespace ui
{
namespace api
{
/**
* \brief ComboBox GUI Widget
*/
class ComboBoxGUI : public InputGUI
{
    Q_OBJECT

public:
    /**
    * \brief ComboBoxGUI constructor
    * \param string gui label
    * \param parent qwidget parent
    */
    ComboBoxGUI(std::string label, QWidget* parent = nullptr);

    /**
    * \brief TextGUI destructor
    */
    ~ComboBoxGUI();

    /**
    * \brief Add callback for activated signal
    * \param LuaRef lua callback
    */
    void addCallback(kaguya::LuaRef cb);

    /**
    * \brief Overridden Set ComboBox GUI widget label
    * \param string gui label
    */
    void setLabel(const std::string& newLabel) override;

    /**
    * \brief Add ComboBox string item
    * \param string item
    * \param int index to insert at, default=end oflist
    */
    void addItem(const std::string& item, int index=-1);

    /**
    * \brief Return selected item string
    * \return string value
    */
    std::string value() const;

    /**
    * \brief Set selected item
    * \param string value
    */
    void setValue(const std::string& val);

    /**
    * \brief Set selected item
    * \param int index of item
    */
    void setValue(int index);

    /**
    * \brief Return lua value
    * \param LuaRef value
    */
    void getLuaValue(kaguya::LuaRef& table) override;

    /**
    * \brief Hide widget label
    */
    void hideLabel() override;

    /**
    * \brief Remove horizontal stretch
    */
    void removeStretch();

public slots:
    /**
    * \brief Run callbacks
    * \param int item index
    */
    void activatedCallbacks(int index);

protected:
    /**
    * \brief Copy widget value to the clipboard
    */
    void copyValue(QDataStream& stream) override;

    /**
    * \brief Set widget value from clipboard
    */
    void pasteValue(QDataStream& stream) override;

private:
    Ui::ComboBoxGUI* ui;
    QLabel* _textLabel;
    QComboBox* _comboBox;

    std::vector<kaguya::LuaRef> _activatedCallbacks;
};
}
}
}
