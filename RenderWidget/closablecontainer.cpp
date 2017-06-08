#include "closablecontainer.h"
#include "global.h"

ClosableContainer::ClosableContainer(QWidget *parent) : QWidget(parent) {
    setLayout(mMainLayout);
    mContentArrow = new QPushButton(QIcon(":/icons/list_show_children.png"),
                                    "",
                                    this);
    mContentArrow->setCheckable(true);
    mContentArrow->setStyleSheet("background: transparent;"
                                 "border: 0;");
    mContentArrow->setIconSize(QSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT));
    mContentArrow->setFixedSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
    connect(mContentArrow, SIGNAL(toggled(bool)),
            this, SLOT(setContentVisible(bool)));

    mMainLayout->addWidget(mContentArrow, Qt::AlignTop);
    mMainLayout->addLayout(mContLayout);
    mMainLayout->setAlignment(mContentArrow, Qt::AlignTop);
}

void ClosableContainer::setLabelWidget(QWidget *widget) {
    mLabelWidget = widget;
    mContLayout->insertWidget(0, widget);
}

void ClosableContainer::setContentWidget(QWidget *widget) {
    mContWidget = widget;
    mContLayout->insertWidget(1, widget);
    mContWidget->setVisible(mContentArrow->isChecked());
}

void ClosableContainer::setContentVisible(const bool &bT) {
    if(bT) {
        mContentArrow->setIcon(QIcon(":/icons/list_hide_children.png"));
    } else {
        mContentArrow->setIcon(QIcon(":/icons/list_show_children.png"));
    }
    if(mContWidget != NULL) {
        mContWidget->setVisible(bT);
    }
}
