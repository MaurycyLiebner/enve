#include "closablecontainer.h"
#include "global.h"
#include <QCheckBox>

ClosableContainer::ClosableContainer(QWidget *parent) : QWidget(parent) {
    setLayout(mMainLayout);
    mContentArrow = new QPushButton(QIcon(":/icons/list_show_children.png"),
                                    "",
                                    this);
    mContentArrow->setObjectName("iconButton");
    mContentArrow->setCheckable(true);
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

void ClosableContainer::addContentWidget(QWidget *widget) {
    mContWidgets << widget;
    mContLayout->insertWidget(mContLayout->count(), widget);
    widget->setVisible(mContentArrow->isChecked());
}

void ClosableContainer::setCheckable(const bool check) {
    if(check == (mCheckBox != nullptr)) return;
    if(check) {
        mCheckBox = new QCheckBox(this);
        mCheckBox->setFixedSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
        mMainLayout->insertWidget(0, mCheckBox, Qt::AlignTop);
        mMainLayout->setAlignment(mCheckBox, Qt::AlignTop);
        mCheckBox->setChecked(true);
    } else {
        delete mCheckBox;
        mCheckBox = nullptr;
    }
}

void ClosableContainer::setChecked(const bool check) {
    if(!mCheckBox) return;
    if(mCheckBox->isChecked() == check) return;
    mCheckBox->setChecked(true);
}

bool ClosableContainer::isChecked() {
    if(!mCheckBox) return true;
    return mCheckBox->isChecked();
}

void ClosableContainer::setContentVisible(const bool bT) {
    if(bT) {
        mContentArrow->setIcon(QIcon(":/icons/list_hide_children.png"));
    } else {
        mContentArrow->setIcon(QIcon(":/icons/list_show_children.png"));
    }
    for(QWidget *widget : mContWidgets) {
        widget->setVisible(bT);
    }
}
