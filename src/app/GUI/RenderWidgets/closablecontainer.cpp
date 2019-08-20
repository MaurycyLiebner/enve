#include "closablecontainer.h"
#include "GUI/global.h"
#include "esettings.h"
#include <QCheckBox>

ClosableContainer::ClosableContainer(QWidget *parent) : QWidget(parent) {
    mMainLayout->setAlignment(Qt::AlignTop);
    setLayout(mMainLayout);
    mContentArrow = new QPushButton("", this);
    mContentArrow->setObjectName("iconButton");
    mContentArrow->setCheckable(true);
    mContentArrow->setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
    mContentArrow->setIconSize(QSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM));
    connect(mContentArrow, &QPushButton::toggled,
            this, &ClosableContainer::setContentVisible);

    mMainLayout->addWidget(mContentArrow, 0, Qt::AlignTop);
    mMainLayout->addLayout(mVLayout);
    mVLayout->setAlignment(Qt::AlignTop);
    mVLayout->addWidget(mContWidget);
    mContWidget->setLayout(mContLayout);
    mContLayout->setAlignment(Qt::AlignTop);
    mVLayout->setSpacing(0);
    mVLayout->setMargin(0);
    setContentVisible(false);
}

void ClosableContainer::setLabelWidget(QWidget *widget) {
    if(mLabelWidget) delete mLabelWidget;
    mLabelWidget = widget;
    if(widget) mVLayout->insertWidget(0, widget);
}

void ClosableContainer::addContentWidget(QWidget *widget) {
    mContWidgets << widget;
    mContLayout->addWidget(widget);
}

void ClosableContainer::setCheckable(const bool check) {
    if(check == bool(mCheckBox)) return;
    if(check) {
        mCheckBox = new QCheckBox(this);
        mCheckBox->setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
        mMainLayout->insertWidget(0, mCheckBox, 0, Qt::AlignTop);
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

void ClosableContainer::setContentVisible(const bool visible) {
    QString iconPath = eSettings::sIconsDir();
    if(visible) iconPath += "/down-arrow.png";
    else iconPath += "/right-arrow.png";
    mContentArrow->setIcon(QIcon(iconPath));
    mContWidget->setVisible(visible);
}
