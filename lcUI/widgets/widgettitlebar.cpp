#include "widgettitlebar.h"
#include "layers.h"

using namespace lc;
using namespace lc::ui;
using namespace lc::ui::widgets;

WidgetTitleBar::WidgetTitleBar( const QString& title,
								QDockWidget* parent,
								WidgetTitleBar::TitleBarOptions hideOptions)
	: QWidget(parent), m_pMainHLayout(nullptr), m_pMainVLayout(nullptr)
{
	pDock = parent;

	this->hideOptions = hideOptions;

	m_pLabel = new QLabel(title, this);
	m_pRLabel = new RotatableLabel(title, this);
	m_pExpandButton = new QPushButton(this);
	m_pCloseButton = new QPushButton(this);

	if (hideOptions == WidgetTitleBar::TitleBarOptions::VerticalOnHidden)
	{
		setHorizontalLayout();
	}
	else if(hideOptions == WidgetTitleBar::TitleBarOptions::HorizontalOnHidden)
	{
		setVerticalLayout();
	}

	// add appropriate icons for close and expand button
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

	if (hideOptions == WidgetTitleBar::TitleBarOptions::VerticalOnHidden)
	{
		setHorizontalLayout();
	}
	else if (hideOptions == WidgetTitleBar::TitleBarOptions::HorizontalOnHidden)
	{
		setVerticalLayout();
	}

    pDock->widget()->setMinimumWidth(_oldSize.width());
	pDock->widget()->show();
    pDock->widget()->setMinimumWidth(0);
}

void WidgetTitleBar::closeButtonTriggered()
{
	m_pCloseButton->hide();
	m_pExpandButton->show();

	if (hideOptions == WidgetTitleBar::TitleBarOptions::VerticalOnHidden)
	{
		setVerticalLayout();
	}
	else if (hideOptions == WidgetTitleBar::TitleBarOptions::HorizontalOnHidden)
	{
		setHorizontalLayout();
	}

    _oldSize = pDock->widget()->size();
	pDock->close();
}

void WidgetTitleBar::setHorizontalLayout()
{
	if (m_pMainVLayout != nullptr)
	{
		// Removes the widgets and destroys v layout
		m_pMainVLayout->removeWidget(m_pRLabel);
		m_pMainVLayout->removeWidget(m_pCloseButton);
		m_pMainVLayout->removeWidget(m_pExpandButton);

		delete m_pMainVLayout;
		m_pMainVLayout = nullptr;
	}

	// Initializes and adds widgets to h layout
	m_pMainHLayout = new QHBoxLayout(this);
	m_pMainHLayout->addWidget(m_pLabel);
	m_pMainHLayout->addWidget(m_pCloseButton);
	m_pMainHLayout->addWidget(m_pExpandButton);
	m_pLabel->show();
	m_pRLabel->hide();

	// disable DockWidgetVerticalTitleBar feature
	pDock->setFeatures(pDock->features() & ~QDockWidget::DockWidgetVerticalTitleBar);
}

void WidgetTitleBar::setVerticalLayout()
{
	if (m_pMainHLayout != nullptr)
	{
		// Removes the widgets and destroys h layout
		m_pMainHLayout->removeWidget(m_pLabel);
		m_pMainHLayout->removeWidget(m_pCloseButton);
		m_pMainHLayout->removeWidget(m_pExpandButton);

		delete m_pMainHLayout;
		m_pMainHLayout = nullptr;
	}

	// Initializes and adds widgets to v layout
	m_pMainVLayout = new QVBoxLayout(this);
	m_pMainVLayout->addWidget(m_pRLabel);
	m_pMainVLayout->addWidget(m_pCloseButton);
	m_pMainVLayout->addWidget(m_pExpandButton);
	m_pRLabel->show();
	m_pLabel->hide();

	// enable DockWidgetVerticalTitleBar feature
	pDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar);
}
