// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#include "actionbutton.h"
#include "GUI/global.h"
#include <QPainter>

ActionButton::ActionButton(const QString &notCheckedPix,
                           const QString &toolTip,
                           QWidget *parent) :
    QWidget(parent) {
    setFocusPolicy(Qt::NoFocus);
    setToolTip(toolTip);
    setFixedSize(BUTTON_DIM, BUTTON_DIM);
    setIcon(notCheckedPix);
}

void ActionButton::setIcon(const QString &notCheckedPix) {
    mNotCheckedPixmap.load(notCheckedPix);
    update();
}

void ActionButton::setCheckable(const QString &checkedPix) {
    mCheckable = true;
    mCheckedPixmap.load(checkedPix);
}

void ActionButton::mousePressEvent(QMouseEvent *) {
    toggle();
    emit pressed();
}

void ActionButton::enterEvent(QEvent *) {
    mHover = true;
    update();
}

void ActionButton::leaveEvent(QEvent *) {
    mHover = false;
    update();
}

void ActionButton::paintEvent(QPaintEvent *) {
    QPainter p(this);

    if(!isEnabled()) p.setOpacity(0.5);
    if(mChecked) p.drawImage(0, 0, mCheckedPixmap);
    else p.drawImage(0, 0, mNotCheckedPixmap);
    if(isEnabled()) {
        if(mHover) {
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(QPen(QColor(255, 255, 255, 125), 2));
            p.drawRoundedRect(1, 1, width() - 2, height() - 2, 2.5, 2.5);
        }
    }

    p.end();
}

void ActionButton::setChecked(bool checked) {
    mChecked = checked;
    update();
}

void ActionButton::toggle() {
    if(mCheckable) {
        setChecked(!mChecked);
        emit toggled(mChecked);
    }
}

void ActionButton::mouseReleaseEvent(QMouseEvent *) {
    emit released();
}
