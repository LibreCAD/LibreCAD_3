#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

namespace lc {
	namespace ui {
		namespace widgets {
			/**
			 * \brief Widget which shows a list of layers
			 */
			class WidgetTitleBar : public QWidget {
				Q_OBJECT

			public:
				WidgetTitleBar(const QString& title, QDockWidget* parent, bool verticalOnHidden);
			protected slots:
				void expandButtonTriggered();
				void closeButtonTriggered();
			private:
				QDockWidget* pDock;
				QVBoxLayout* m_pMainVLayout;
				QHBoxLayout* m_pMainHLayout;
				QLabel* m_pLabel;
				QPushButton* m_pExpandButton;
				QPushButton* m_pCloseButton;
				QStackedWidget* stackedWidget;
				bool verticalOnHidden;
				void setHorizontalLayout(bool switched);
				void setVerticalLayout(bool switched);
			};
		}
	}
}