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
