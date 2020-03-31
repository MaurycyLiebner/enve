// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "arraywidget.h"
#include <QStyle>
#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>

ArrayWidget::ArrayWidget(const Qt::Orientation& orientation,
                         QWidget *parent) : QWidget(parent) {
    setFocusPolicy(Qt::NoFocus);

    mPrevButton = new QPushButton("", this);
    mNextButton = new QPushButton("", this);
    mPrevButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    mNextButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    setOrientation(orientation);

    connect(mNextButton, &QPushButton::pressed,
            this, &ArrayWidget::nextButtonPressed);
    connect(mPrevButton, &QPushButton::pressed,
            this, &ArrayWidget::prevButtonPressed);

    updateButtonIcons();
}

void ArrayWidget::setOrientation(const Qt::Orientation &orientation) {
    if(mMainLayout) {
        delete mMainLayout;
    }

    mOrientation = orientation;
    if(orientation == Qt::Vertical) {
        mMainLayout = new QVBoxLayout(this);
        mMainLayout->addWidget(mPrevButton);
        mMainLayout->addWidget(mNextButton);
        mMainLayout->setAlignment(mPrevButton, Qt::AlignBottom);
        mMainLayout->setAlignment(mNextButton, Qt::AlignTop);
    } else {
        mMainLayout = new QHBoxLayout(this);
        mMainLayout->addWidget(mPrevButton);
        mMainLayout->addWidget(mNextButton);
        mMainLayout->setAlignment(mPrevButton, Qt::AlignRight);
        mMainLayout->setAlignment(mNextButton, Qt::AlignLeft);
    }
    setLayout(mMainLayout);

    updateWidgetsVisibility();
    updateButtonIcons();
}

void ArrayWidget::appendWidget(QWidget *wid) {
    insertWidget(mArrayWidgets.size(), wid);
}

void ArrayWidget::prependWidget(QWidget *wid) {
    insertWidget(0, wid);
}

void ArrayWidget::insertWidget(const int i, QWidget *wid) {
    wid->setParent(this);
    mMainLayout->insertWidget(i + 1, wid);
    mArrayWidgets.insert(i, wid);
    connectChildWidget(wid);
    removeRedundantWidgets();
    updateWidgetsVisibility();
}

void ArrayWidget::removeRedundantWidgets() {
    while(mArrayWidgets.count() > mMaxCount) {
        removeWidget(mArrayWidgets.last());
    }
}

bool ArrayWidget::removeWidget(QWidget *widget) {
    for(QWidget* wid : mArrayWidgets) {
        if(wid == widget) {
            mArrayWidgets.removeOne(wid);
            delete wid;
            return true;
        }
    }
    return false;
}

void ArrayWidget::incFirstWidgetId(const int inc) {
    setFirstWidgetId(mFirstId + inc);
}

void ArrayWidget::decFirstWidgetId(const int dec) {
    setFirstWidgetId(mFirstId - dec);
}

void ArrayWidget::setFirstWidgetId(const int id) {
    mFirstId = id;
    updateWidgetsVisibility();
}

void ArrayWidget::setMaxCount(const int maxCount) {
    mMaxCount = std::max(0, maxCount);
}

void ArrayWidget::updateWidgetsVisibility() {
    int i = 0;
    for(; i < mArrayWidgets.count() && i < mFirstId; i++) {
        mArrayWidgets.at(i)->hide();
    }
    int shown = 0;
    for(; i < mArrayWidgets.count() && shown < mMaxShownCount; i++) {
        mArrayWidgets.at(i)->show();
        shown++;
    }
    mPrevButton->setDisabled(mFirstId <= 0);
    mNextButton->setDisabled(i >= mArrayWidgets.count());
    for(; i < mArrayWidgets.count(); i++) {
        mArrayWidgets.at(i)->hide();
    }
}

void ArrayWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateWidgetsVisibility();
}

void ArrayWidget::connectChildWidget(QWidget *wid) {
    Q_UNUSED(wid)
}

void ArrayWidget::nextButtonPressed() {
    incFirstWidgetId();
}

void ArrayWidget::prevButtonPressed() {
    decFirstWidgetId();
}

void ArrayWidget::updateButtonIcons() {
    QIcon nextIcon;
    QIcon prevIcon;
    if(mOrientation == Qt::Horizontal) {
        nextIcon = QIcon(QApplication::style()->standardPixmap(QStyle::SP_ArrowRight));
        prevIcon = QIcon(QApplication::style()->standardPixmap(QStyle::SP_ArrowLeft));
    } else {
        nextIcon = QIcon(QApplication::style()->standardPixmap(QStyle::SP_ArrowDown));
        prevIcon = QIcon(QApplication::style()->standardPixmap(QStyle::SP_ArrowUp));
    }
    mNextButton->setIcon(nextIcon);
    mPrevButton->setIcon(prevIcon);
}
