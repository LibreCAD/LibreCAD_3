#pragma once

#include <QListWidget>
#include "inputgui.h"
#include <mainwindow.h>

#include <cad/builders/lwpolyline.h>

namespace Ui {
class ListGUI;
}

namespace lc
{
namespace ui
{
namespace api
{
/**
* \brief List GUI Widget
*/
class ListGUI : public InputGUI
{
    Q_OBJECT

public:
    enum class ListType
    {
        NONE,
        COORDINATE,
        LW_VERTEX
    };

public:
    /**
    * \brief ListGUI constructor
    * \param string gui label
    * \param ListType listType
    * \param parent qwidget parent
    */
    ListGUI(std::string label, ListGUI::ListType listType = ListGUI::ListType::NONE, QWidget* parent = nullptr);

    /**
    * \brief ListGUI destructor
    */
    ~ListGUI();

    /**
    * \brief Get lua value
    * \param LuaRef table
    */
    void getLuaValue(kaguya::LuaRef& table) override;

    /**
    * \brief Add list item to list
    * \param string key
    * \param pointer to InputGUI new item
    */
    void addItem(const std::string& key, InputGUI* newitem);

    /**
    * \brief Connect required signals to mainwindow (in case of coordinate,etc)
    * \param pointer to MainWindow
    */
    void setMainWindow(lc::ui::MainWindow* mainWindowIn);

    /**
    * \brief Set label of the list gui widget
    * \param string new label
    */
    void setLabel(const std::string& newLabel) override;

    /**
    * \brief Set list type to a given type (all existing list items should be of same type)
    * \param ListType list type
    */
    void setListType(ListGUI::ListType listType);

    /**
    * \brief Set list type to a given type through string (for lua)
    * \param string list type
    */
    void setListType(const std::string& listTypeStr);

    /**
    * \brief Set callback for all items in the list
    * \param LuaRef lua callback
    */
    void addCallbackToAll(kaguya::LuaRef cb);

    /**
    * \brief Add given coordinates as list items
    * \param vector of coordinates
    */
    void setValue(std::vector<lc::geo::Coordinate> coords);

    /**
    * \brief Add given polyline vertices as list items
    * \param vector of polyline builder vertices
    */
    void setValue(std::vector<lc::builder::LWBuilderVertex> builderVertices);

    /**
    * \brief Return list type of list
    * \return ListType list type
    */
    ListType listType() const;

    /**
    * \brief Return keys of all list items
    * \return set of list key strings
    */
    std::set<std::string> getKeys() const;

    /**
    * \brief Hide widget label
    */
    void hideLabel() override;

public slots:
    /**
    * \brief Slot for the plus add button being clicked (button is disabled for NONE list type)
    */
    void plusButtonClicked();

    /**
    * \brief Slot for the minus delete button being clicked (button is disabled for NONE list type)
    */
    void minusButtonClicked();

    /**
    * \brief Slot for current item selection being changed
    */
    void guiItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

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
    Ui::ListGUI* ui;
    QListWidget* listWidget;
    std::vector<InputGUI*> itemList;
    std::set<std::string> _addedKeys;
    lc::ui::MainWindow* mainWindow;
    ListType _listType;
    std::vector<kaguya::LuaRef> _callbacks;
    unsigned int itemIdCount;
    lc::entity::CADEntity_CSPtr _selectedCoordinate;
};

}
}
}
