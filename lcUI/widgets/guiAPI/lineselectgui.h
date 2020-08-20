#pragma once

#include "inputgui.h"
#include <widgets/linewidthselect.h>
#include <widgets/linepatternselect.h>
#include <widgets/colorselect.h>

namespace lc {
namespace ui {
namespace api {
/**
* \brief Line Select GUI class
*/
class LineSelectGUI : public InputGUI
{
    Q_OBJECT

public:
    /**
    * \brief Line Select GUI constructor
    * \param pointer to CadMdiChild
    * \param pointer to MetaInfoManager
    * \param string label
    */
    LineSelectGUI(CadMdiChild* mdiChild, MetaInfoManager_SPtr metaInfoManager, std::string label, QWidget* parent = nullptr);

    /**
    * \brief Add lua value to table (not used)
    * \param LuaRef value table
    */
    void getLuaValue(kaguya::LuaRef& table) override;

    /**
    * \brief Hide label for the widget (not used)
    */
    void hideLabel() override;

    /**
    * \brief Add lua callback
    * \param LuaRef callback
    */
    void addCallback(kaguya::LuaRef cb);

    /**
    * \brief Set select tools to current meta info of the given entity
    * \param pointer to CADEntity
    */
    void setEntityMetaInfo(lc::entity::CADEntity_CSPtr entity);

public slots:
    /**
    * \brief Metea info manager changed
    */
    void metaInfoManagerChanged();

protected:
    void copyValue(QDataStream& stream) override;

    void pasteValue(QDataStream& stream) override;

private:
    lc::ui::widgets::LinePatternSelect _linePatternSelect;
    lc::ui::widgets::LineWidthSelect _lineWidthSelect;
    lc::ui::widgets::ColorSelect _colorSelect;
    std::vector<kaguya::LuaRef> _callbacks;
};
}
}
}
