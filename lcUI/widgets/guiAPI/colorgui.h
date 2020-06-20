#pragma once

#include <string>

#include "inputgui.h"
#include <cad/meta/color.h>

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace Ui {
    class ColorGUI;
}

namespace lc {
    namespace ui {
        namespace api {
            class ColorGUI : public InputGUI
            {
                Q_OBJECT

            public:
                ColorGUI(std::string label, QWidget* parent = nullptr);

                ~ColorGUI();

                void getLuaValue(kaguya::LuaRef& table);

                lc::Color value() const;

                void setValue(lc::Color col);

                void addCallback(kaguya::LuaRef cb);

                void colorSelectedCallbacks();

            public slots:
                void changeColor();

            private:
                QColor getIdealTextColor(const QColor& rBackgroundColor) const;

            private:
                Ui::ColorGUI* ui;
                QLabel* _textLabel;
                QPushButton* _colorButton;
                QColor _color;

                std::vector<kaguya::LuaRef> _callbacks;
            };

        }
    }
}
