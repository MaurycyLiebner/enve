#include "boundingbox.h"
#include "boxeslist.h"
#include "boxesgroup.h"
#include "undoredo.h"
#include <QPainter>

QrealKey *BoundingBox::getKeyAtPos(qreal relX, qreal relY, qreal) {
    if(relY < 0.) return NULL;
    int minViewedFrame = mBoxesList->getMinViewedFrame();
    int pixelsPerFrame =  mBoxesList->getPixelsPerFrame();
    QrealKey *collectionKey = NULL;
    qreal currY = relY;
    if(currY <= LIST_ITEM_HEIGHT) {
        collectionKey = mAnimatorsCollection.getKeyAtPos(relX,
                               minViewedFrame,
                               pixelsPerFrame);
    }
    currY -= LIST_ITEM_HEIGHT;
    if(collectionKey == NULL) {
        foreach(QrealAnimator *animator, mActiveAnimators) {
            if(currY <= LIST_ITEM_HEIGHT) {
                QrealKey *animatorKey = animator->getKeyAtPos(relX,
                                       minViewedFrame,
                                       pixelsPerFrame);
                if(animatorKey == NULL) continue;
                return animatorKey;
            }
            currY -= LIST_ITEM_HEIGHT;
        }
    } else {
        return collectionKey;
    }
    return NULL;
}

void BoundingBox::handleListItemMousePress(qreal relX, qreal relY) {
    Q_UNUSED(relY);
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
            scheduleBoxesListRepaint();
            scheduleRepaint();
        }
    }
}

void BoundingBox::getKeysInRect(QRectF selectionRect, qreal y0,
                                QList<QrealKey*> *keysList) {
    qreal rectMargin = (LIST_ITEM_HEIGHT - KEY_RECT_SIZE)*0.5;
    int minViewedFrame = mBoxesList->getMinViewedFrame();
    int pixelsPerFrame =  mBoxesList->getPixelsPerFrame();
    if(selectionRect.bottom() > y0 && selectionRect.top() < y0) {
        mAnimatorsCollection.getKeysInRect(selectionRect,
                                         minViewedFrame,
                                         pixelsPerFrame,
                                         keysList);
    }
    y0 += LIST_ITEM_HEIGHT;
    foreach(QrealAnimator *animator, mActiveAnimators) {
        if(y0 + rectMargin > selectionRect.bottom() ) return;
        if(y0 + LIST_ITEM_HEIGHT - rectMargin > selectionRect.top() ) {
            animator->getKeysInRect(selectionRect,
                                   minViewedFrame,
                                   pixelsPerFrame,
                                   keysList);
        }
        y0 += LIST_ITEM_HEIGHT;
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

void BoxesGroup::getKeysInRectFromChildren(QRectF selectionRect, qreal y0,
                                           QList<QrealKey *> *keysList) {
    qreal currentY = y0;
    foreach(BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        QRectF boxRect = QRectF(selectionRect.left(), currentY,
                                selectionRect.width(), boxHeight);
        if(boxRect.intersects(selectionRect)) {
            box->getKeysInRect(selectionRect,
                               currentY, keysList);
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::getKeysInRect(QRectF selectionRect, qreal y0,
                                QList<QrealKey*> *keysList) {
    BoundingBox::getKeysInRect(selectionRect, y0, keysList);
    getKeysInRectFromChildren(selectionRect, y0 +
                              (1 + mActiveAnimators.length())*LIST_ITEM_HEIGHT,
                              keysList);
}

void Canvas::getKeysInRect(QRectF selectionRect, qreal y0,
                           QList<QrealKey *> *keysList) {
    getKeysInRectFromChildren(selectionRect, y0, keysList);
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
    return getKeyAtPosFromChildren(relX, relY, y0);
}

QrealKey *Canvas::getKeyAtPos(qreal relX, qreal relY,
                               qreal y0) {
    return getKeyAtPosFromChildren(relX, relY, y0);
}

void BoxesGroup::handleChildListItemMousePress(qreal relX, qreal relY,
                                               qreal y0) {
    qreal currentY = y0;
    foreach(BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(relY - currentY < boxHeight) {
            box->handleListItemMousePress(relX, relY - currentY);
            break;
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::handleListItemMousePress(qreal relX, qreal relY) {
    if(relY < LIST_ITEM_HEIGHT) {
        BoundingBox::handleListItemMousePress(relX, relY);
    } else {
        handleChildListItemMousePress(relX - LIST_ITEM_HEIGHT, relY,
                                      LIST_ITEM_HEIGHT);
    }
}

void BoundingBox::drawListItem(QPainter *p,
                               qreal drawX, qreal drawY,
                               qreal maxY, qreal pixelsPerFrame,
                               int startFrame, int endFrame) {
    Q_UNUSED(maxY);
    drawAnimationBar(p, pixelsPerFrame,
                     200., drawY,
                     startFrame, endFrame);

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
                                   int startFrame, int endFrame) {
    mAnimatorsCollection.drawKeys(p,
                                pixelsPerFrame, drawX, drawY, 20.,
                                startFrame, endFrame, true);
    drawY += LIST_ITEM_HEIGHT;
    if(mBoxListItemDetailsVisible) {
        foreach(QrealAnimator *animator, mActiveAnimators) {
            animator->drawKeys(p, pixelsPerFrame, drawX, drawY, 20.,
                               startFrame, endFrame, true);
            drawY += LIST_ITEM_HEIGHT;
        }
    }
}

void BoxesGroup::drawChildren(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal minY, qreal maxY, qreal pixelsPerFrame,
                              int startFrame, int endFrame) {
    qreal currentY = drawY;
    qreal currentDrawY = drawY;
    foreach (BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(currentY >= minY) {
            if(currentY > maxY) {
                break;
            }
            box->drawListItem(p, drawX, currentDrawY, maxY,
                              pixelsPerFrame, startFrame, endFrame);
            currentDrawY += boxHeight;
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::drawListItem(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal maxY, qreal pixelsPerFrame,
                              int startFrame, int endFrame)
{
    BoundingBox::drawListItem(p, drawX, drawY, maxY,
                              pixelsPerFrame, startFrame, endFrame);
    if(mBoxListItemDetailsVisible) {
        drawX += LIST_ITEM_CHILD_INDENT;
        drawY += LIST_ITEM_HEIGHT*(1 + mActiveAnimators.length());
        drawChildren(p, drawX, drawY, 0.f, maxY,
                     pixelsPerFrame, startFrame, endFrame);
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
    return LIST_ITEM_HEIGHT + ( (mBoxListItemDetailsVisible) ?
                LIST_ITEM_HEIGHT*mActiveAnimators.length() : 0);
}

void BoundingBox::setChildrenListItemsVisible(bool bt)
{
    mBoxListItemDetailsVisible = bt;
    scheduleBoxesListRepaint();
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
    scheduleBoxesListRepaint();
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
    scheduleBoxesListRepaint();
    scheduleRepaint();
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
    scheduleBoxesListRepaint();
}
