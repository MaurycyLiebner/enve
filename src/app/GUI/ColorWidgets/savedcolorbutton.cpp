#include "savedcolorbutton.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include "global.h"

SavedColorButton::SavedColorButton(const QColor &colorT,
                                   QWidget *parent) :
    QWidget(parent) {
    mColor = colorT;
    setFixedSize(2*MIN_WIDGET_DIM, 2*MIN_WIDGET_DIM);
}

void SavedColorButton::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton) {
        emit colorButtonPressed(mColor);
    } else if(e->button() == Qt::RightButton) {
        QMenu menu(this);
        menu.addAction("Delete Color");
        QAction *selected_action = menu.exec(e->globalPos());
        if(selected_action != nullptr) {
            if(selected_action->text() == "Delete Color") {
                deleteLater();
            }
        } else {

        }
    }
}

void SavedColorButton::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), mColor);
    p.end();
}

