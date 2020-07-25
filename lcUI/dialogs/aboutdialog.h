#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QLabel>

namespace lc {
    namespace ui {
        namespace dialog {
            class AboutDialog : public QDialog {
                Q_OBJECT

                public:
                    AboutDialog(QWidget* parent = 0);
		    QString _info;
                private slots:
		    void copyClick();
            };
        }
    }
}
