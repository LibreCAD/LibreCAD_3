#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStylePainter>
#include<iostream>

namespace lc {
	namespace ui {
		namespace widgets {

			class RotatableLabel : public QLabel
			{
			Q_OBJECT

			public:
				explicit RotatableLabel(QWidget* parent = 0)
					:
					QLabel(parent),
					angle(90)
				{}
				explicit RotatableLabel(const QString& text, QWidget* parent = 0)
					:
					QLabel(text, parent),
					angle(90)
				{}
				void SetRotateAngle(float rot)
				{
					angle = rot;
				}

			protected:
				void paintEvent(QPaintEvent* event)
				{
					QPainter painter(this);
					painter.setPen(Qt::black);
					painter.setBrush(Qt::Dense1Pattern);

					painter.rotate(angle);
						
					// 5,-5
					painter.drawText(0,0, text());
				}
				QSize RotatableLabel::minimumSizeHint() const
				{
					QSize s = QLabel::minimumSizeHint();
					return QSize(s.height(), s.width());
				}

				QSize RotatableLabel::sizeHint() const
				{
					QSize s = QLabel::sizeHint();
					return QSize(s.height(), s.width());
				}
			private:
				float angle;
			};
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
				RotatableLabel* m_pRLabel;
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