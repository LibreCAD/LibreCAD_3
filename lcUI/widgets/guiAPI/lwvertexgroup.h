#pragma once

#include "horizontalgroupgui.h"
#include "coordinategui.h"
#include "numbergui.h"
#include <mainwindow.h>

namespace lc
{
    namespace ui
    {
        namespace api
        {
            /**
            * \brief Custom LWVertex Group GUI Widget
            */
            class LWVertexGroup : public HorizontalGroupGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief LWVertexGroup constructor
                * \param string gui label
                * \param parent qwidget parent
                */
                LWVertexGroup(std::string label, QWidget* parent = nullptr);

                /**
                * \brief Add lua callback
                * \param LuaRef callback
                */
                void addCallback(kaguya::LuaRef cb);

                /**
                * \brief Set main window i.e. enable coordinate selection
                * \param Pointer to mainwindow
                */
                void setMainWindow(lc::ui::MainWindow* mainWindowIn);

                /**
                * \brief Store the value in the lua table
                * \param LuaRef to table
                */
                void getLuaValue(kaguya::LuaRef& table) override;

                /**
                * \brief Set the vertex group key
                * \param string key
                */
                void setKey(const std::string& keyIn) override;

                /**
                * \brief Set the value of the vertex group
                * \param geo::Coordinate coord
                * \param double StartWidth
                * \param double EndWidth
                * \param double Bulge
                */
                void setValue(lc::geo::Coordinate coord, double sw, double ew, double bulge);

                /**
                * \brief Get the current location value
                * \return geo::Coordinate location
                */
                lc::geo::Coordinate location() const;

            private:
                lc::ui::api::CoordinateGUI* coordgui;
                lc::ui::api::NumberGUI* startWidth;
                lc::ui::api::NumberGUI* endWidth;
                double _bulge;
            };
        }
    }
}
