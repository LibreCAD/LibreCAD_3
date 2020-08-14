#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStylePainter>
#include <QSize>

namespace lc {
	namespace ui {
		namespace widgets {

			/**
			 * @brief RotatableLabel (by default rotates by 90)
			 */
			class RotatableLabel : public QLabel
			{
			Q_OBJECT

			public:
				explicit RotatableLabel(QWidget* parent = 0)
					:
					QLabel(parent),
					angle(90),
					qfm(QFont("times", 16))
				{}
				explicit RotatableLabel(const QString& text, QWidget* parent = 0)
					:
					QLabel(text, parent),
					angle(90),
					qfm(QFont("times", 16))
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
						
					// hardcoded to 10 and -10, TODO think of a way to dynamically get values
					painter.drawText(10,-10, text());
				}
				QSize minimumSizeHint() const
				{
					return QSize(qfm.height(), qfm.boundingRect(text()).width());
				}

				QSize sizeHint() const
				{
					return QSize(qfm.height(), qfm.boundingRect(text()).width());
				}
			private:
				float angle;
				// QFontMetrics allows greater control plus fixed a bug of some
				// of the text being covered by the button
				QFontMetrics qfm;
			};

			/**
			 * @brief Custom widget title bar to replace default one
			 *
			 * Requires parent widget to override closeEvent and ignore event to stop
			 * the widget from closing (see layers.cpp for example)
			 */
			class WidgetTitleBar : public QWidget {
				Q_OBJECT
	
			public:
				enum class TitleBarOptions {
					VerticalOnHidden,
					HorizontalOnHidden
				};

			public:
				/**
				 * @params title of widget, parent, boolean for title bar to be vertical on hidden
				 */
				WidgetTitleBar( const QString& title,
								QDockWidget* parent,
								const TitleBarOptions hideOptions);

                void setTitleBarOrientation(TitleBarOptions options);

            public slots:
                void determineTitleBarOrientation();

			protected slots:
				void expandButtonTriggered();
				void closeButtonTriggered();

			private:
				QDockWidget* pDock;

				QVBoxLayout* m_pMainVLayout;
				QHBoxLayout* m_pMainHLayout;

				// RotatableLabel for vertical text
				RotatableLabel* m_pRLabel;
				// Normal Qlabel for horizontal text
				QLabel* m_pLabel;

				QPushButton* m_pExpandButton;
				QPushButton* m_pCloseButton;

				// enums for hide options
				TitleBarOptions hideOptions;
                QSize _oldSize;

                QDockWidget::DockWidgetFeatures expandedFeatures;
                bool closed;

				// functions to switch to horizontal or vertical layouts
				void setHorizontalLayout();
				void setVerticalLayout();
			};
		}
	}
}
