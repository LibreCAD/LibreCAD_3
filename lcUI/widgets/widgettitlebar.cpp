#include "widgettitlebar.h"
#include "layers.h"

using namespace lc;
using namespace lc::ui;
using namespace lc::ui::widgets;

WidgetTitleBar::WidgetTitleBar(const QString& title, QDockWidget* parent, bool verticalOnHidden)
	: QWidget(parent)
{
	pDock = parent;

	this->verticalOnHidden = verticalOnHidden;

	m_pLabel = new QLabel(title, this);
	m_pExpandButton = new QPushButton(this);
	m_pCloseButton = new QPushButton(this);

	if (verticalOnHidden)
	{
		m_pMainHLayout = new QHBoxLayout(this);

		m_pMainHLayout->addWidget(m_pLabel);
		m_pMainHLayout->addWidget(m_pExpandButton);
		m_pMainHLayout->addWidget(m_pCloseButton);
	}
	else
	{
		m_pMainVLayout = new QVBoxLayout(this);

		m_pMainVLayout->addWidget(m_pLabel);
		m_pMainVLayout->addWidget(m_pExpandButton);
		m_pMainVLayout->addWidget(m_pCloseButton);
	}

	QIcon icon1 = pDock->style()->standardIcon(QStyle::SP_TitleBarMaxButton, 0, pDock);
	QIcon icon2 = pDock->style()->standardIcon(QStyle::SP_TitleBarCloseButton, 0, pDock);

	m_pExpandButton->setIcon(icon1);
	m_pCloseButton->setIcon(icon2);

	m_pExpandButton->hide();

	connect(m_pExpandButton, SIGNAL(clicked()), this, SLOT(expandButtonTriggered()));
	connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(closeButtonTriggered()));
}

void WidgetTitleBar::expandButtonTriggered()
{
	m_pCloseButton->show();
	m_pExpandButton->hide();

	setHorizontalLayout(!verticalOnHidden);

	pDock->widget()->show();
	pDock->setFeatures(pDock->features() & ~QDockWidget::DockWidgetVerticalTitleBar);
}

void WidgetTitleBar::closeButtonTriggered()
{
	m_pCloseButton->hide();
	m_pExpandButton->show();

	setVerticalLayout(!verticalOnHidden);
	pDock->close();
}

void WidgetTitleBar::setHorizontalLayout(bool switched)
{
	if (switched)
	{
		setVerticalLayout(!switched);
		return;
	}

	m_pMainVLayout->removeWidget(m_pLabel);
	m_pMainVLayout->removeWidget(m_pCloseButton);
	m_pMainVLayout->removeWidget(m_pExpandButton);

	delete m_pMainVLayout;
	m_pMainVLayout = nullptr;

	m_pMainHLayout = new QHBoxLayout(this);
	m_pMainHLayout->addWidget(m_pLabel);
	m_pMainHLayout->addWidget(m_pCloseButton);
	m_pMainHLayout->addWidget(m_pExpandButton);
}

void WidgetTitleBar::setVerticalLayout(bool switched)
{
	if (switched)
	{
		setHorizontalLayout(!switched);
		return;
	}

	m_pMainHLayout->removeWidget(m_pLabel);
	m_pMainHLayout->removeWidget(m_pCloseButton);
	m_pMainHLayout->removeWidget(m_pExpandButton);

	delete m_pMainHLayout;
	m_pMainHLayout = nullptr;

	m_pMainVLayout = new QVBoxLayout(this);
	m_pMainVLayout->addWidget(m_pLabel);
	m_pMainVLayout->addWidget(m_pCloseButton);
	m_pMainVLayout->addWidget(m_pExpandButton);
}