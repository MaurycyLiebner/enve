#include "boxeslist.h"
#include "canvas.h"
#include <QPainter>
#include <QResizeEvent>
#include "mainwindow.h"
#include "qrealkey.h"

ChangeWidthWidget::ChangeWidthWidget(QWidget *boxesList, QWidget *parent) :
    QWidget(parent) {
    mBoxesList = boxesList;
    setFixedWidth(10);
    setFixedHeight(4000);
    setCursor(Qt::SplitHCursor);
}

void ChangeWidthWidget::updatePos()
{
    move(mBoxesList->width() - 5, 0);
}

void ChangeWidthWidget::mouseMoveEvent(QMouseEvent *event)
{
    int newWidth = mBoxesList->width() + event->x() - mPressX;
    newWidth = qMax(200, newWidth);
    BoxesListWidget::setListItemMaxWidth(newWidth);
    mBoxesList->setFixedWidth(newWidth);
    updatePos();
}

void ChangeWidthWidget::mousePressEvent(QMouseEvent *event)
{
    mPressX = event->x();
}
