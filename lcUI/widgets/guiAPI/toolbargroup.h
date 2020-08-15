#pragma once

#include <QGroupBox>

#include <kaguya/kaguya.hpp>
#include "toolbarbutton.h"

namespace lc
{
namespace ui
{
namespace api
{
/**
 * \brief Widget used as group for grouping toolbar buttons
 */
class ToolbarGroup : public QGroupBox
{
    Q_OBJECT

public:
    /**
    * \brief ToolbarGroup Constructor
    * \param groupName string group name
    * \param width int max buttons in one row
    */
    ToolbarGroup(const char* groupName, int width = 3, QWidget* parent = nullptr);

    /**
    * \brief Add toolbar button to group
    * \param ToolbarButton button
    */
    void addButton(ToolbarButton* button);

    /**
    * \brief Add toolbar button to group
    * \param string button name
    * \param string button icon
    * \return pointer to toolbar button
    */
    ToolbarButton* addButton(const char* buttonName, const char* buttonIcon);

    /**
    * \brief Add toolbar button to group
    * \param string button name
    * \param string button icon
    * \param LuaRef button callback
    * \return pointer to toolbar button
    */
    ToolbarButton* addButton(const char* buttonName, const char* buttonIcon, kaguya::LuaRef callback);

    /**
    * \brief Add non button widget to group
    * \param pointer to qwidget
    */
    void addWidget(QWidget* widget);

    /**
    * \brief get label
    * \return string label
    */
    std::string label() const;

    /**
    * \brief Set group label
    * \param string new group name
    */
    void setLabel(const char* newGroupName);

    /**
    * \brief Get button in group
    * \param string button name
    * \return pointer to toolbar button
    */
    ToolbarButton* buttonByName(const char* buttonName);

    /**
    * \brief Get all buttons in the group
    * \return vector of toolbar button pointers
    */
    std::vector<ToolbarButton*> buttons();

    /**
    * \brief Remove button in group
    * \param string button name
    */
    void removeButton(const char* buttonName);

    /**
    * \brief Remove this group
    */
    void remove();

    /**
    * \brief Get width of group
    * \return int width
    */
    int width() const;

    /**
    * \brief Change number of columns for group
    */
    void setWidth(int width);

    /**
    * \brief Don't show group in customize toolbar widget
    */
    void setNonButtonGroup(bool nonButtonGroupIn);

    /**
    * \brief Is the group a non button group
    */
    bool nonButtonGroup() const;

    /**
    * \brief Removes all buttons from group (but don't delete button) and remove parentship
    */
    void clear();

signals:
    /**
    * \brief Signal for group remove
    */
    void removeGroup(ToolbarGroup* group);

public slots:
    /**
    * \brief Slot to remove button
    * \param pointer to toolbar button
    */
    void removeButton(ToolbarButton* button);

private:
    int _width;
    int _count;
    bool _nonButtonGroup;
};
}
}
}
