#pragma once

#include "inputgui.h"
#include <QWidget>
#include <QSlider>
#include <QLabel>

namespace Ui {
class SliderGUI;
}

namespace lc
{
namespace ui
{
namespace api
{
/**
* \brief Slider GUI Widget
*/
class SliderGUI : public InputGUI
{
    Q_OBJECT

public:
    /**
    * \brief SliderGUI constructor
    * \param string gui label
    * \param set minimum value int
    * \param set maximum value int
    * \param parent qwidget parent
    */
    SliderGUI(std::string label, int setMin=0, int setMax=100, QWidget* parent = nullptr);

    /**
    * \brief SliderGUI destructor
    */
    ~SliderGUI();

    /**
    * \brief Add callback for value changed signal
    * \param LuaRef lua callback
    */
    void addCallback(kaguya::LuaRef cb);

    /**
    * \brief Overriden set SLider GUI widget label
    * \param string gui label
    */
    void setLabel(const std::string& newLabel) override;

    /**
    * \brief Return slider value
    * \return int value
    */
    int value() const;

    /**
    * \brief Set slider value
    * \param int value
    */
    void setValue(int val);

    /**
    * \brief Return lua value
    * \param LuaRef value
    */
    void getLuaValue(kaguya::LuaRef& table) override;

    /**
    * \brief Hide widget label
    */
    void hideLabel() override;

public slots:
    /**
    * \brief Run callbacks for value changed
    * \param int new value
    */
    void valueChangedCallbacks(int value);

    /**
    * \brief Update display label to show new value
    * \param int new value
    */
    void updateValueDisplay(int value);

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
    Ui::SliderGUI* ui;
    QLabel* _textLabel;
    QSlider* _slider;
    QLabel* _displayValue;

    std::vector<kaguya::LuaRef> _valueChangeCallbacks;
};
}
}
}
