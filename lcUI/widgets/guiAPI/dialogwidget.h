#pragma once

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <set>

#include <kaguya/kaguya.hpp>
#include "inputguicontainer.h"
#include "inputgui.h"
#include "buttongui.h"
#include "checkboxgui.h"

#include "mainwindow.h"

namespace Ui { 
    class DialogWidget; 
}

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Dialog Widget
            */
            class DialogWidget : public QDialog, public InputGUIContainer
            {
                Q_OBJECT

            public:
                /**
                * \brief Dialog Widget Constructor
                * \param string dialog title
                * \param parent qwidget parent
                */
                DialogWidget(const std::string& dialogTitle, lc::ui::MainWindow* mainWindow, QWidget* parent = nullptr);

                /**
                * \brief Dialog Widget Destructor
                */
                ~DialogWidget();

                /**
                * \brief Add input gui widget
                * \param pointer to InputGUI widget
                * \return successfully added bool
                */
                bool addWidget(const std::string& key, InputGUI* guiWidget) override;

                /**
                * \brief Add button directly (create a button group containing only that button)
                * \param pointer to ButtonGUI button
                * \return successfully added bool
                */
                bool addWidget(const std::string& key, ButtonGUI* buttonWidget) override;

                /**
                * \brief Add checkbox directly (create a button group containing only that button)
                * \param pointer to CheckBoxGUI button
                * \return successfully added bool
                */
                bool addWidget(const std::string& key, CheckBoxGUI* checkboxWidget) override;

                /**
                * \brief Set title of dialog widget
                * \param string new title
                */
                void setTitle(const std::string& newTitle);

                /**
                * \brief Return title of the dialog widget
                * \return string dialog title
                */
                std::string title() const;

                /**
                * \brief Set button to be connected to finish callbacks
                * \return pointer to ButtonGUI buttonWidget
                */
                void setFinishButton(ButtonGUI* buttonWidget);

            public slots:
                /**
                * \brief Call Lua callbacks to be called on finish
                */
                void finishCallbacks();

            private:
                Ui::DialogWidget* ui;
                QVBoxLayout* vboxlayout;
            };
        }
    }
}
