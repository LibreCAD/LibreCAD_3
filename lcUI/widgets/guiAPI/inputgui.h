#pragma once

#include <QObject>
#include <QMetaObject>
#include <QWidget>
#include <QContextMenuEvent>

// Phase 4 PR-5b — kaguya removed from the InputGUI base header.  Subclass
// getValue(Map&) writes into a neutral ScriptValue::Map that adapters
// materialize per-runtime.
#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

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
    * \brief Write the widget's key/value into a language-neutral Map that
    * the callback runtime will materialize into its own table shape
    * (Lua table, Python dict).
    * \param ScriptValue::Map neutral table (shared_ptr — aliasing-safe;
    *        see the ListGUI aliasing note in scriptvalue.h).
    */
    virtual void getValue(lc::scripting::Map& map) = 0;

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
