#include "closablecontainer.h"
#include "GUI/global.h"
#include "esettings.h"
#include <QCheckBox>

ClosableContainer::ClosableContainer(QWidget *parent) : QWidget(parent) {
    setLayout(mMainLayout);
    const auto iconPath = eSettings::sIconsDir() + "/right-arrrow.png";
    mContentArrow = new QPushButton(QIcon(iconPath), "", this);
    mContentArrow->setObjectName("iconButton");
    mContentArrow->setCheckable(true);
    mContentArrow->setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
    mContentArrow->setIconSize(QSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM));
    connect(mContentArrow, &QPushButton::toggled,
            this, &ClosableContainer::setContentVisible);

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
        mCheckBox->setFixedSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
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
        mContentArrow->setIcon(QIcon(eSettings::sIconsDir() + "/down-arrow.png"));
    } else {
        mContentArrow->setIcon(QIcon(eSettings::sIconsDir() + "/right-arrow.png"));
    }
    for(const auto widget : mContWidgets) widget->setVisible(bT);
}
