#include "widgetcontainer.h"
#include "boxeslistwidget.h"

WidgetContainer::WidgetContainer(QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    mAllLayout = new QVBoxLayout(this);
    mAllLayout->setAlignment(Qt::AlignTop);
    mAllLayout->setSpacing(0);
    mAllLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mAllLayout);
    //mAllLayout->addWidget(mTargetBoxWidget);
    mDetailsWidget = new QWidget(this);
    mChildWidgetsLayout = new QVBoxLayout(mDetailsWidget);
    mChildWidgetsLayout->setAlignment(Qt::AlignTop);
    mChildWidgetsLayout->setSpacing(0);
    mChildWidgetsLayout->setContentsMargins(BoxesListWidget::getListItemChildIndent(), 0, 0, 0);
    mDetailsWidget->setLayout(mChildWidgetsLayout);
    mAllLayout->addWidget(mDetailsWidget);

    mDetailsWidget->hide();
}

void WidgetContainer::setDetailsVisible(bool visible)
{
    mDetailsWidget->setVisible(visible);
}

void WidgetContainer::setTopWidget(QWidget *widget) {
    mAllLayout->insertWidget(0, widget);
}

void WidgetContainer::addChildWidget(QWidget *widget) {
    mChildWidgetsLayout->addWidget(widget);
}
