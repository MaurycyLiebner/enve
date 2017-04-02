#include "actionbutton.h"
#include <QPainter>

ActionButton::ActionButton(const QString &notCheckedPix,
                           const QString &toolTip,
                           QWidget *parent) :
    QWidget(parent) {
    setToolTip(toolTip);
    setFixedSize(24, 24);
    mNotCheckedPixmap.load(notCheckedPix);
}

void ActionButton::setCheckable(const QString &checkedPix) {
    mCheckable = true;
    mCheckedPixmap.load(checkedPix);
}

void ActionButton::mousePressEvent(QMouseEvent *) {
    if(mCheckable) {
        setChecked(!mChecked);
        emit toggled(mChecked);
    }
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

void ActionButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if(mChecked) {
        p.drawImage(0, 0, mCheckedPixmap);
    } else {
        p.drawImage(0, 0, mNotCheckedPixmap);
    }
    if(mHover) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(255, 255, 255, 125), 2.));
        p.drawRoundedRect(1., 1., 22., 22., 2.5, 2.5);
    }

    p.end();
}

void ActionButton::setChecked(bool checked) {
    mChecked = checked;
    update();
}
