#include "boundingbox.h"
#include "boxeslist.h"
#include "boxesgroup.h"
#include "undoredo.h"
#include <QPainter>

QrealKey *BoundingBox::getKeyAtPos(qreal relX, qreal relY, qreal) {
    if(relY < 0.) return NULL;
    int minViewedFrame = mBoxesList->getMinViewedFrame();
    qreal pixelsPerFrame =  mBoxesList->getPixelsPerFrame();
    QrealKey *collectionKey = NULL;
    qreal currY = relY;
    if(currY <= LIST_ITEM_HEIGHT) {
        collectionKey = mAnimatorsCollection.getKeyAtPos(relX, relY,
                               minViewedFrame,
                               pixelsPerFrame);
    }
    currY -= LIST_ITEM_HEIGHT;
    if(collectionKey == NULL) {
        foreach(QrealAnimator *animator, mActiveAnimators) {
            qreal animatorHeight = animator->getBoxesListHeight();
            if(currY <= animatorHeight) {
                QrealKey *animatorKey = animator->getKeyAtPos(relX, currY,
                                       minViewedFrame,
                                       pixelsPerFrame);
                if(animatorKey == NULL) continue;
                return animatorKey;
            }
            currY -= animatorHeight;
        }
    } else {
        return collectionKey;
    }
    return NULL;
}

void BoundingBox::handleListItemMousePress(qreal boxesListX,
                                           qreal relX, qreal relY,
                                           QMouseEvent *event) {
    if(relX < 0) {
        return;
    }
    if(relY < LIST_ITEM_HEIGHT) {
        if(event->button() == Qt::LeftButton) {
            if(relX < LIST_ITEM_HEIGHT) {
                setChildrenListItemsVisible(!mBoxListItemDetailsVisible);
            } else if(relX < LIST_ITEM_HEIGHT*2) {
                setVisibile(!mVisible);
            } else if(relX < LIST_ITEM_HEIGHT*3) {
                setLocked(!mLocked);
            } else {
                if(isVisibleAndUnlocked() && mParent->isCurrentGroup()) {
                    if(isShiftPressed()) {
                        if(mSelected) {
                            mParent->removeBoxFromSelection(this);
                        } else {
                            mParent->addBoxToSelection(this);
                        }
                    } else {
                        mParent->clearBoxesSelection();
                        mParent->addBoxToSelection(this);
                    }
                }
            }
        } else if(event->button() == Qt::RightButton) {
            showContextMenu(event->globalPos() );
        }
    } else if(mBoxListItemDetailsVisible) {
        relY -= LIST_ITEM_HEIGHT;
        foreach(QrealAnimator *animator, mActiveAnimators) {
            qreal animatorHeight = animator->getBoxesListHeight();
            if(relY <= animatorHeight) {
                animator->handleListItemMousePress(boxesListX,
                                                   relX - LIST_ITEM_CHILD_INDENT,
                                                   relY, event);
                break;
            }
            relY -= animatorHeight;
        }
    }
}

void BoundingBox::getKeysInRect(QRectF selectionRect,
                                QList<QrealKey*> *keysList) {
    qreal rectMargin = (LIST_ITEM_HEIGHT - KEY_RECT_SIZE)*0.5;
    if(selectionRect.bottom() < rectMargin) return;
    int minViewedFrame = mBoxesList->getMinViewedFrame();
    qreal pixelsPerFrame =  mBoxesList->getPixelsPerFrame();
    if(selectionRect.top() < LIST_ITEM_HEIGHT - rectMargin) {
        mAnimatorsCollection.getKeysInRect(selectionRect,
                                         minViewedFrame,
                                         pixelsPerFrame,
                                         keysList);
    }
    selectionRect.translate(0., -LIST_ITEM_HEIGHT);
    foreach(QrealAnimator *animator, mActiveAnimators) {
        if(selectionRect.bottom() < rectMargin) return;
        qreal heightT = animator->getBoxesListHeight();
        if(heightT - rectMargin > selectionRect.top() ) {
            animator->getKeysInRect(selectionRect,
                                   minViewedFrame,
                                   pixelsPerFrame,
                                   keysList);
        }
        selectionRect.translate(0., -heightT);
    }
}

void BoundingBox::addActiveAnimator(QrealAnimator *animator)
{
    mActiveAnimators << animator;
}

void BoundingBox::removeActiveAnimator(QrealAnimator *animator)
{
    mActiveAnimators.removeOne(animator);
}

void BoxesGroup::getKeysInRectFromChildren(QRectF selectionRect,
                                           QList<QrealKey *> *keysList) {
    foreach(BoundingBox *box, mChildren) {
        if(selectionRect.bottom() < 0.) return;
        qreal boxHeight = box->getListItemHeight();
        QRectF boxRect = QRectF(selectionRect.left(), 0.,
                                selectionRect.width(), boxHeight);
        if(boxRect.intersects(selectionRect)) {
            box->getKeysInRect(selectionRect, keysList);
        }
        selectionRect.translate(0., -boxHeight);
    }
}

void BoxesGroup::getKeysInRect(QRectF selectionRect,
                                QList<QrealKey*> *keysList) {
    BoundingBox::getKeysInRect(selectionRect, keysList);
    qreal heightT = BoundingBox::getListItemHeight();
    getKeysInRectFromChildren(selectionRect.translated(0., -heightT),
                              keysList);
}

void Canvas::getKeysInRect(QRectF selectionRect,
                           QList<QrealKey *> *keysList) {
    getKeysInRectFromChildren(selectionRect, keysList);
}

QrealKey *BoxesGroup::getKeyAtPosFromChildren(qreal relX, qreal relY,
                               qreal y0) {
    qreal currentY = y0;
    foreach(BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(relY - currentY < boxHeight) {
            return box->getKeyAtPos(relX, relY - currentY, currentY);
        }
        currentY += boxHeight;
    }
    return NULL;
}

QrealKey *BoxesGroup::getKeyAtPos(qreal relX, qreal relY,
                               qreal y0) {
    QrealKey *keyFromThis = BoundingBox::getKeyAtPos(relX, relY, y0);
    if(keyFromThis != NULL) return keyFromThis;
    return getKeyAtPosFromChildren(relX, relY, BoundingBox::getListItemHeight());
}

QrealKey *Canvas::getKeyAtPos(qreal relX, qreal relY,
                               qreal y0) {
    return getKeyAtPosFromChildren(relX, relY, y0);
}

void BoxesGroup::handleChildListItemMousePress(qreal boxesListX,
                                               qreal relX, qreal relY,
                                               qreal y0, QMouseEvent *event) {
    qreal currentY = y0;
    foreach(BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(relY - currentY < boxHeight) {
            box->handleListItemMousePress(boxesListX,
                                          relX, relY - currentY,
                                          event);
            break;
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::handleListItemMousePress(qreal boxesListX,
                                          qreal relX, qreal relY,
                                          QMouseEvent *event) {
    qreal heightT = BoundingBox::getListItemHeight();
    if(relY < heightT) {
        BoundingBox::handleListItemMousePress(boxesListX,
                                              relX, relY, event);
    } else {
        handleChildListItemMousePress(boxesListX,
                                      relX - LIST_ITEM_HEIGHT, relY,
                                      heightT, event);
    }
}

void BoundingBox::drawListItem(QPainter *p,
                               qreal drawX, qreal drawY,
                               qreal maxY, qreal pixelsPerFrame,
                               int startFrame, int endFrame,
                               bool animationBar) {
    Q_UNUSED(maxY);
    drawAnimationBar(p, pixelsPerFrame,
                     drawX + LIST_ITEM_HEIGHT, drawY,
                     startFrame, endFrame, animationBar);

    if(mSelected) {
        p->setBrush(QColor(185, 185, 185));
    } else {
        p->setBrush(QColor(225, 225, 225));
    }
    p->drawRect(QRectF(drawX, drawY,
                       LIST_ITEM_MAX_WIDTH - drawX, LIST_ITEM_HEIGHT));

    if(mBoxListItemDetailsVisible) {
        p->drawPixmap(drawX, drawY, *BoxesList::HIDE_CHILDREN);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::SHOW_CHILDREN);
    }
    drawX += LIST_ITEM_HEIGHT;
    if(mVisible) {
        p->drawPixmap(drawX, drawY, *BoxesList::VISIBLE_PIXMAP);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::INVISIBLE_PIXMAP);
    }
    drawX += LIST_ITEM_HEIGHT;
    if(mLocked) {
        p->drawPixmap(drawX, drawY, *BoxesList::LOCKED_PIXMAP);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::UNLOCKED_PIXMAP);
    }
    drawX += 2*LIST_ITEM_HEIGHT;
    p->drawText(QRectF(drawX, drawY,
                       LIST_ITEM_MAX_WIDTH - drawX - LIST_ITEM_HEIGHT,
                       LIST_ITEM_HEIGHT),
                mName, QTextOption(Qt::AlignVCenter));
}

void BoundingBox::drawAnimationBar(QPainter *p,
                                   qreal pixelsPerFrame,
                                   qreal drawX, qreal drawY,
                                   int startFrame, int endFrame,
                                   bool animationBar) {
    if(animationBar) {
        mAnimatorsCollection.drawKeys(p,
                                    pixelsPerFrame, LIST_ITEM_MAX_WIDTH, drawY, 20.,
                                    startFrame, endFrame, true);
    }
    drawY += LIST_ITEM_HEIGHT;
    if(mBoxListItemDetailsVisible) {
        foreach(QrealAnimator *animator, mActiveAnimators) {
            animator->drawBoxesList(p, drawX, drawY,
                                    pixelsPerFrame,
                                    startFrame, endFrame,
                                    animationBar);
            drawY += animator->getBoxesListHeight();
        }
    }
}

void BoxesGroup::drawChildrenListItems(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal maxY, qreal pixelsPerFrame,
                              int startFrame, int endFrame,
                              bool animationBar) {
    qreal currentY = drawY;
    foreach (BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(currentY + boxHeight >= 0.) {
            if(currentY > maxY) {
                break;
            }
            box->drawListItem(p, drawX, currentY, maxY,
                              pixelsPerFrame, startFrame, endFrame,
                              animationBar);
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::drawListItem(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal maxY, qreal pixelsPerFrame,
                              int startFrame, int endFrame,
                              bool animationBar)
{
    BoundingBox::drawListItem(p, drawX, drawY, maxY,
                              pixelsPerFrame, startFrame, endFrame,
                              animationBar);
    if(mBoxListItemDetailsVisible) {
        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += BoundingBox::getListItemHeight();
        drawChildrenListItems(p, drawX, drawY, maxY,
                              pixelsPerFrame, startFrame, endFrame,
                              animationBar);
    }
}

qreal BoxesGroup::getListItemHeight() {
    qreal height = BoundingBox::getListItemHeight();
    if(mBoxListItemDetailsVisible) {
        foreach(BoundingBox *box, mChildren) {
            height += box->getListItemHeight();
        }
    }
    return height;
}

qreal BoundingBox::getListItemHeight() {
    if(mBoxListItemDetailsVisible) {
        qreal height = LIST_ITEM_HEIGHT;
        foreach(QrealAnimator *animator, mActiveAnimators) {
            height += animator->getBoxesListHeight();
        }

        return height;
    } else {
        return LIST_ITEM_HEIGHT;
    }
}

void BoundingBox::setChildrenListItemsVisible(bool bt)
{
    mBoxListItemDetailsVisible = bt;
}

void BoundingBox::showChildrenListItems()
{
    setChildrenListItemsVisible(true);
}

void BoundingBox::hideChildrenListItems()
{
    setChildrenListItemsVisible(false);
}

void BoundingBox::setName(QString name)
{
    mName = name;
}

QString BoundingBox::getName()
{
    return mName;
}

void BoundingBox::setVisibile(bool visible, bool saveUndoRedo) {
    if(mVisible == visible) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxVisibleUndoRedo(this, mVisible, visible));
    }
    mVisible = visible;
}

void BoundingBox::hide()
{
    setVisibile(false);
}

void BoundingBox::show()
{
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() {
    return mVisible && !mLocked;
}

bool BoundingBox::isVisible()
{
    return mVisible;
}

bool BoundingBox::isLocked() {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(bool bt) {
    if(bt == mLocked) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    mLocked = bt;
}
