#include "boxtargetwidget.h"
#include <QMimeData>
#include <QPainter>
#include "Boxes/boundingbox.h"
#include "Properties/boxtargetproperty.h"
#include "mainwindow.h"

BoxTargetWidget::BoxTargetWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
    setMaximumWidth(150);
}

void BoxTargetWidget::setTargetProperty(BoxTargetProperty *property) {
    mProperty = property;
    update();
}

void BoxTargetWidget::dropEvent(
        QDropEvent *event) {
    if(event->mimeData()->hasFormat("boundingbox")) {
        mProperty->setTarget(((BoundingBoxMimeData*)event->mimeData())->
                getBoundingBox());
        MainWindow::getInstance()->callUpdateSchedulers();
        mDragging = false;
        update();
    }
}

void BoxTargetWidget::dragEnterEvent(
        QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat("boundingbox")) {
        event->acceptProposedAction();
        mDragging = true;
        update();
    }
}

void BoxTargetWidget::dragLeaveEvent(
        QDragLeaveEvent *) {
    mDragging = false;
    update();
}

void BoxTargetWidget::paintEvent(QPaintEvent *) {
    if(mProperty == NULL) return;
    QPainter p(this);
    if(mProperty->SWT_isDisabled()) p.setOpacity(.5);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    if(mDragging) {
        p.setPen(Qt::white);
    } else {
        p.setPen(Qt::black);
    }
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5., 5.);

    p.setPen(Qt::black);
    if(mProperty) {
        BoundingBox *target = mProperty->getTarget();
        if(target == NULL) {
            p.drawText(rect(), Qt::AlignCenter, "-none-");
        } else {
            p.drawText(rect(), Qt::AlignCenter, target->getName());
        }
    }

    p.end();
}
