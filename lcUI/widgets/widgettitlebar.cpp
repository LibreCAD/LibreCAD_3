#include "widgettitlebar.h"
#include "layers.h"

#include <QMainWindow>

using namespace lc;
using namespace lc::ui;
using namespace lc::ui::widgets;

WidgetTitleBar::WidgetTitleBar( const QString& title,
								QDockWidget* parent,
								WidgetTitleBar::TitleBarOptions hideOptions)
	: QWidget(parent), m_pMainHLayout(nullptr), m_pMainVLayout(nullptr), closed(false)
{
	pDock = parent;
    expandedFeatures = pDock->features();

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

    connect(pDock, &QDockWidget::dockLocationChanged, this, &widgets::WidgetTitleBar::determineTitleBarOrientation);
}

void WidgetTitleBar::expandButtonTriggered()
{
    closed = false;
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

    pDock->setMinimumWidth(_oldSize.width());
	pDock->widget()->show();
    pDock->setMinimumWidth(0);
}

void WidgetTitleBar::closeButtonTriggered()
{
    closed = true;
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

    _oldSize = pDock->size();
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
    if (closed) {
        pDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    }
    else {
        pDock->setFeatures(expandedFeatures & ~QDockWidget::DockWidgetVerticalTitleBar);
    }
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
    if (closed) {
        pDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar);
    }
    else {
        pDock->setFeatures(expandedFeatures | QDockWidget::DockWidgetVerticalTitleBar);
    }
}

void WidgetTitleBar::determineTitleBarOrientation() {
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pDock->parentWidget());

    if (mainWindow != nullptr) {
        Qt::DockWidgetArea widgetArea = mainWindow->dockWidgetArea(pDock);
        
        if (widgetArea == Qt::TopDockWidgetArea || widgetArea == Qt::BottomDockWidgetArea) {
            setTitleBarOrientation(TitleBarOptions::HorizontalOnHidden);
        }
        else if(widgetArea == Qt::LeftDockWidgetArea || widgetArea == Qt::RightDockWidgetArea){
            setTitleBarOrientation(TitleBarOptions::VerticalOnHidden);
        }
    }
}

void WidgetTitleBar::setTitleBarOrientation(TitleBarOptions options) {
    if (this->hideOptions == options) {
        return;
    }

    this->hideOptions = options;

    if (hideOptions == WidgetTitleBar::TitleBarOptions::VerticalOnHidden)
    {
        setHorizontalLayout();
    }
    else if (hideOptions == WidgetTitleBar::TitleBarOptions::HorizontalOnHidden)
    {
        setVerticalLayout();
    }
}
