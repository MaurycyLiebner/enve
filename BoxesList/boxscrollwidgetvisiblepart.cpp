#include "boxscrollwidgetvisiblepart.h"
#include "boxsinglewidget.h"
#include <QPainter>
#include "Animators/qrealanimator.h"
#include "boxscrollwidget.h"
#include <QTimer>
#include <QMimeData>
#include "Boxes/boundingbox.h"
#include "Boxes/boxesgroup.h"
#include "mainwindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"

BoxScrollWidgetVisiblePart::BoxScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    ScrollWidgetVisiblePart(parent) {
    setAcceptDrops(true);
    mScrollTimer = new QTimer(this);
}

SingleWidget *BoxScrollWidgetVisiblePart::createNewSingleWidget() {
    return new BoxSingleWidget(this);
}

void BoxScrollWidgetVisiblePart::paintEvent(QPaintEvent *) {
    QPainter p(this);

//    p.fillRect(rect(), Qt::red);
    int currY = BOX_HEIGHT;
    p.setPen(QPen(QColor(40, 40, 40), 1.));
    while(currY < height()) {
        p.drawLine(0, currY, width(), currY);

        currY += BOX_HEIGHT;
    }

    if(mDragging) {
        p.setPen(QPen(QColor(40, 40, 40), 3.));
        p.drawLine(0, mCurrentDragPosId*BOX_HEIGHT,
                   width(), mCurrentDragPosId*BOX_HEIGHT);
    }

    p.end();
}

void BoxScrollWidgetVisiblePart::drawKeys(QPainter *p,
             const qreal &pixelsPerFrame,
             const int &minViewedFrame, const int &maxViewedFrame) {
    p->setPen(QPen(Qt::black, 1.));
    foreach(SingleWidget *container, mSingleWidgets) {
        ((BoxSingleWidget*)container)->drawKeys(
                            p, pixelsPerFrame,
                            container->y(),
                            minViewedFrame, maxViewedFrame);
    }
}

QrealKey *BoxScrollWidgetVisiblePart::getKeyAtPos(
        const int &pressX, const int &pressY,
        const qreal &pixelsPerFrame,
        const int &minViewedFrame) {
    int remaining = pressY % BOX_HEIGHT;
    if(remaining < (BOX_HEIGHT - KEY_RECT_SIZE)/2 ||
       remaining > (BOX_HEIGHT + KEY_RECT_SIZE)/2) return NULL;
    foreach(SingleWidget *container, mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > pressY || containerBottom < pressY) continue;
        return ((BoxSingleWidget*)container)->
                getKeyAtPos(pressX, pixelsPerFrame,
                            minViewedFrame);
    }
    return NULL;
}

void BoxScrollWidgetVisiblePart::getKeysInRect(
        QRectF selectionRect,
        qreal pixelsPerFrame,
        const int &minViewedFrame,
        QList<QrealKey*> *listKeys) {
    selectionRect.adjust(0, (BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5,
                         0, -(BOX_HEIGHT/* + KEY_RECT_SIZE*/)*0.5);
    foreach(SingleWidget *container, mSingleWidgets) {
        int containerTop = container->y();
        int containerBottom = containerTop + container->height();
        if(containerTop > selectionRect.bottom() ||
           containerBottom < selectionRect.top()) continue;
        ((BoxSingleWidget*)container)->
                getKeysInRect(selectionRect,
                              pixelsPerFrame,
                              minViewedFrame,
                              listKeys);
    }
}

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetTypeLookBelow(
                const SWT_Type &type,
                const int &yPos,
                bool *isBelow) {
    int idAtYPos = yPos / 20;
    int targetId = (yPos + 10) / 20;
    if(idAtYPos < mSingleWidgets.count() &&
       idAtYPos >= 0) {
        BoxSingleWidget *singleWidgetUnderMouse = (BoxSingleWidget*)
                mSingleWidgets.at(
                    idAtYPos);
        while(singleWidgetUnderMouse->isHidden()) {
            idAtYPos++;
            if(idAtYPos >= mSingleWidgets.count()) return NULL;
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        while(singleWidgetUnderMouse->getTargetAbstraction()->
              getTarget()->SWT_getType() != type) {
            idAtYPos++;
            if(idAtYPos >= mSingleWidgets.count()) return NULL;
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        *isBelow = targetId > idAtYPos;
        return singleWidgetUnderMouse;
    }
    return NULL;
}

BoxSingleWidget *BoxScrollWidgetVisiblePart::
            getClosestsSingleWidgetWithTargetType(
                const SWT_Type &type,
                const int &yPos,
                bool *isBelow) {
    int idAtYPos = yPos / 20;
    int targetId = (yPos + 10) / 20;
    if(idAtYPos < mSingleWidgets.count() &&
       idAtYPos >= 0) {
        BoxSingleWidget *singleWidgetUnderMouse = (BoxSingleWidget*)
                mSingleWidgets.at(
                    idAtYPos);
        while(singleWidgetUnderMouse->isHidden()) {
            idAtYPos--;
            if(idAtYPos < 0) {
                return getClosestsSingleWidgetWithTargetTypeLookBelow(
                            type, yPos, isBelow);
            }
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        while(singleWidgetUnderMouse->getTargetAbstraction()->
              getTarget()->SWT_getType() != type) {
            idAtYPos--;
            if(idAtYPos < 0) {
                return getClosestsSingleWidgetWithTargetTypeLookBelow(
                            type, yPos, isBelow);
            }
            singleWidgetUnderMouse = (BoxSingleWidget*)
                    mSingleWidgets.at(
                        idAtYPos);
        }
        *isBelow = targetId > idAtYPos;
        return singleWidgetUnderMouse;
    }
    return NULL;
}

void BoxScrollWidgetVisiblePart::dropEvent(
        QDropEvent *event) {
    if(event->mimeData()->hasFormat("boundingbox")) {
        int yPos = event->pos().y();
        bool below;
        BoxSingleWidget *singleWidgetUnderMouse =
                getClosestsSingleWidgetWithTargetType(SWT_BoundingBox,
                                                      yPos,
                                                      &below);
        if(singleWidgetUnderMouse == NULL) return;

        BoundingBox *box = ((BoundingBoxMimeData*)event->mimeData())->
                getBoundingBox();
        BoundingBox *boxUnderMouse =
                ((BoundingBox*)singleWidgetUnderMouse->
                 getTargetAbstraction()->getTarget());

        BoxesGroup *parentGroup = boxUnderMouse->getParent();
        if(parentGroup == NULL ||
           boxUnderMouse->isAncestor(box)) return;
        if(parentGroup != box->getParent()) {
            box->getParent()->removeChild(box);
            parentGroup->addChild(box);
        }
        if(below) { // add box below
            parentGroup->moveChildAbove( // boxesgroup list is reversed
                        box,
                       boxUnderMouse);
        } else { // add box above
            parentGroup->moveChildBelow(
                        box,
                        boxUnderMouse);
        }
    }
    mDragging = false;
    scheduledUpdateVisibleWidgetsContent();
    MainWindow::getInstance()->callUpdateSchedulers();
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
}

void BoxScrollWidgetVisiblePart::dragEnterEvent(
        QDragEnterEvent *event)
{
    //mDragging = true;
    if(event->mimeData()->hasFormat("boundingbox")) {
        event->acceptProposedAction();
    }
}

void BoxScrollWidgetVisiblePart::dragLeaveEvent(
        QDragLeaveEvent *) {
    mDragging = false;
    if(mScrollTimer->isActive()) {
        QPoint mousePos = mapFromGlobal(QCursor::pos());
        if(mousePos.x() < 20 || mousePos.x() > width() - 20) {
            mScrollTimer->disconnect();
            mScrollTimer->stop();
        } else {
            mScrollTimer->setInterval(100);
        }
    }
    update();
}

#include <QDebug>
void BoxScrollWidgetVisiblePart::dragMoveEvent(
        QDragMoveEvent *event) {    
    int yPos = event->pos().y();

    if(yPos < 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, SIGNAL(timeout()),
                    this, SLOT(scrollUp()));
            mScrollTimer->start(300);
        }
    } else if(yPos > height() - 30) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, SIGNAL(timeout()),
                    this, SLOT(scrollDown()));
            mScrollTimer->start(300);
        }
    } else {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    mLastDragMoveY = yPos;
    if(event->mimeData()->hasFormat("boundingbox")) {
        mLastDragMoveTargetType = SWT_BoundingBox;
    }

    updateDraggingHighlight();
}

void BoxScrollWidgetVisiblePart::updateDraggingHighlight() {
    mDragging = false;
    bool below;
    BoxSingleWidget *singleWidgetUnderMouse =
            getClosestsSingleWidgetWithTargetType(mLastDragMoveTargetType,
                                                  mLastDragMoveY,
                                                  &below);
    if(singleWidgetUnderMouse != NULL) {
        int currentDragPosId = singleWidgetUnderMouse->y()/20;
        if(below) {
            currentDragPosId++;
        }
        mDragging = true;
        mCurrentDragPosId = currentDragPosId;
    }
    update();
}

void BoxScrollWidgetVisiblePart::scrollUp() {
    mParentWidget->scrollParentAreaBy(-20);
    updateDraggingHighlight();
}

void BoxScrollWidgetVisiblePart::scrollDown() {
    mParentWidget->scrollParentAreaBy(20);
    updateDraggingHighlight();
}
