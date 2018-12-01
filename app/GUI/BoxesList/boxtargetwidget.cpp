#include "boxtargetwidget.h"
#include <QMimeData>
#include <QPainter>
#include "Boxes/boxesgroup.h"
#include "Properties/boxtargetproperty.h"
#include "GUI/mainwindow.h"

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
        auto boxMimeData = static_cast<const BoundingBoxMimeData*>(event->mimeData());
        BoundingBox *targetT = boxMimeData->getTarget();
        mProperty->setTarget(targetT);
        mDragging = false;
        update();
        MainWindow::getInstance()->callUpdateSchedulers();
    }
}

void BoxTargetWidget::dragEnterEvent(
        QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat("boundingbox")) {
        auto boxMimeData = static_cast<const BoundingBoxMimeData*>(event->mimeData());
        BoundingBox *targetT = boxMimeData->getTarget();
        BoundingBox *parentBox =
                mProperty->getLastSetParentBoundingBoxAncestor();
        if(parentBox == targetT) return;
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

void BoxTargetWidget::mousePressEvent(QMouseEvent *event) {
    if(mProperty == nullptr) return;
    if(event->button() == Qt::LeftButton) {
        BoundingBox *parentBox = mProperty->getLastSetParentBoundingBoxAncestor();
        if(parentBox == nullptr) return;
        BoxesGroup *srcGroup = parentBox->getParentGroup();
        if(srcGroup == nullptr) return;
        QList<qsptr<BoundingBox> > boxesT =
                srcGroup->getContainedBoxesList();
        QMenu menu(this);


        BoundingBox *currentTarget = mProperty->getTarget();
        int i = -1;
        QAction *act = menu.addAction("-none-");
        if(currentTarget == nullptr) {
            act->setCheckable(true);
            act->setChecked(true);
            act->setDisabled(true);
        }
        foreach(const qsptr<BoundingBox> &boxT, boxesT) {
            i++;
            if(boxT.data() == parentBox) continue;
            QAction *act2 = menu.addAction(boxT->prp_getName());
            act2->setProperty("targetBoxPtr", i);
            if(currentTarget == boxT.data()) {
                act2->setCheckable(true);
                act2->setChecked(true);
                act2->setDisabled(true);
            }
        }
        QAction *selected_action = menu.exec(mapToGlobal(QPoint(0, height())));
        if(selected_action != nullptr) {
            QVariant varT = selected_action->property("targetBoxPtr");
            if(varT.isValid()) {
                mProperty->setTarget(boxesT.at(varT.toInt()).data());
            } else {
                mProperty->setTarget(nullptr);
            }
            MainWindow::getInstance()->callUpdateSchedulers();
        } else {

        }
    } else if(event->button() == Qt::RightButton) {

    }
}

void BoxTargetWidget::paintEvent(QPaintEvent *) {
    if(mProperty == nullptr) return;
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
        if(target == nullptr) {
            p.drawText(rect(), Qt::AlignCenter, "-none-");
        } else {
            p.drawText(rect(), Qt::AlignCenter, target->getName());
        }
    }

    p.end();
}
