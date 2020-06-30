#pragma once

#include <QWidget>
#include <QLabel>
#include <QDropEvent>

namespace lc {
    namespace ui {
        namespace widgets {
            /**
            * \brief Delete icon area
            */
            class DeleteIconArea : public QLabel
            {
                Q_OBJECT
            public:
                DeleteIconArea(QWidget* parent = nullptr);

            protected:
                void dragEnterEvent(QDragEnterEvent* event) override;
                void dragLeaveEvent(QDragLeaveEvent* event) override;
                void dropEvent(QDropEvent* event) override;
            };
        }
    }
}
