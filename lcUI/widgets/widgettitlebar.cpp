#include "widgettitlebar.h"
#include "layers.h"

using namespace lc;
using namespace lc::ui;
using namespace lc::ui::widgets;

WidgetTitleBar::WidgetTitleBar(const QString& title, QDockWidget* parent)
	: QWidget(parent)
{
	pDock = parent;

	m_pMainVLayout = new QVBoxLayout(this);

	m_pLabel = new QLabel(title, this);
	m_pMainVLayout->addWidget(m_pLabel);

	m_pExpandButton = new QPushButton(this);
	m_pCloseButton = new QPushButton(this);

	QIcon icon1 = pDock->style()->standardIcon(QStyle::SP_TitleBarMaxButton, 0, pDock);
	QIcon icon2 = pDock->style()->standardIcon(QStyle::SP_TitleBarCloseButton, 0, pDock);

	m_pExpandButton->setIcon(icon1);
	m_pCloseButton->setIcon(icon2);

	m_pExpandButton->hide();

	Layers* parentObj = (Layers*)parent;

	connect(m_pExpandButton, SIGNAL(clicked()), parentObj, SLOT(showWidget()));
	connect(m_pExpandButton, SIGNAL(clicked()), this, SLOT(expandButtonTriggered()));
	connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(closeButtonTriggered()));

	m_pMainVLayout->addWidget(m_pExpandButton);
	m_pMainVLayout->addWidget(m_pCloseButton);
}

void WidgetTitleBar::expandButtonTriggered()
{
	m_pCloseButton->show();
	m_pExpandButton->hide();
}

void WidgetTitleBar::closeButtonTriggered()
{
	m_pCloseButton->hide();
	m_pExpandButton->show();
	pDock->close();
}