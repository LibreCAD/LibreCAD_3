#pragma once

#include <QObject>
#include <QMetaObject>
#include <QWidget>
#include <QContextMenuEvent>

#include <kaguya/kaguya.hpp>

// Phase 4 PR-5a — scalar-widget subclasses store callbacks as neutral
// ScriptCallback; kaguya remains needed here for the base
// getLuaValue(LuaRef&) virtual (retired in PR-5b's getValue(Map&) port).
#include <lcscripting/scriptcallback.h>

#include <string>
#include <vector>

namespace lc
{
namespace ui
{
namespace api
{
/**
* \brief Input GUI Parent Widget
*/
class InputGUI : public QWidget
{
    Q_OBJECT

public:
    /**
    * \brief InputGUI constructor
    * \param string gui label
    * \param parent qwidget parent
    */
    InputGUI(std::string _label, QWidget* parent = nullptr);

    /**
    * \brief Get GUI widget label
    * \return string gui label
    */
    std::string label() const;

    /**
    * \brief Set GUI widget label
    * \param string gui label
    */
    virtual void setLabel(const std::string& newLabel);

    /**
    * \brief add key and value to the lua table
    * \param LuaRef lua table
    */
    virtual void getLuaValue(kaguya::LuaRef& table) = 0;

    /**
    * \brief Hide label of the input gui
    */
    virtual void hideLabel() = 0;

    /**
    * \brief copy input gui value to the clipboard event
    */
    void copyEvent();

    /**
    * \brief paste value from clipboard to the input gui event
    */
    void pasteEvent();

    /**
    * \brief Set gui key for the lua table
    * \param string gui key
    */
    virtual void setKey(const std::string& keyIn);

    /**
    * \brief Get input gui key
    * \return string gui key
    */
    const std::string key() const;

protected:
    /**
    * \brief Context menu for input gui
    */
    void contextMenuEvent(QContextMenuEvent* event) override;

    /**
    * \brief Copy widget value to the clipboard
    */
    virtual void copyValue(QDataStream& stream) = 0;

    /**
    * \brief Paste widget value to the clipboard
    */
    virtual void pasteValue(QDataStream& stream) = 0;

    /**
    * \brief Enable or disable menu for the given widgets
    */
    void setCopyPasteEnabled(bool enable);

protected:
    std::string _label;
    std::string _key;
    std::string _type;
    bool _copyPasteEnabled;
};
}
}
}
