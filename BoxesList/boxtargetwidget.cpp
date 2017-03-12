#include "boxtargetwidget.h"
#include <QMimeData>
#include <QPainter>
#include "Boxes/boundingbox.h"
#include "Properties/boxtargetproperty.h"
#include "mainwindow.h"

BoxTargetWidget::BoxTargetWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
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
    QPainter p(this);

    if(mProperty) {
        BoundingBox *target = mProperty->getTarget();
        if(target == NULL) {
            p.drawText(rect(), Qt::AlignCenter, "-none-");
        } else {
            p.drawText(rect(), Qt::AlignCenter, target->getName());
        }
    }
    if(mDragging) {
        p.setPen(Qt::white);
    }
    p.drawRoundedRect(rect(), 2., 2.);

    p.end();
}
