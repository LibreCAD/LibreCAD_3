#pragma once

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>

#include "inputgui.h"

namespace Ui { 
    class DialogWidget; 
}

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Dialog Widget
            */
            class DialogWidget : public QDialog
            {
                Q_OBJECT

            public:
                /**
                * \brief Dialog Widget Constructor
                * \param string dialog title
                * \param parent qwidget parent
                */
                DialogWidget(const std::string& dialogTitle, QWidget* parent = nullptr);

                /**
                * \brief Dialog Widget Destructor
                */
                ~DialogWidget();

                /**
                * \brief Add input gui widget
                * \param pointer to InputGUI widget
                */
                void addWidget(InputGUI* guiWidget);

                /**
                * \brief Return list of all input widgets
                * \return vector of pointer of InputGUI widgets
                */
                const std::vector<InputGUI*>& inputWidgets();

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

            private:
                Ui::DialogWidget* ui;
                QVBoxLayout* vboxlayout;

                std::vector<InputGUI*> _inputWidgets;
            };
        }
    }
}
