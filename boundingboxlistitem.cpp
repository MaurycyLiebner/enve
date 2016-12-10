#include "boundingbox.h"
#include "boxeslist.h"
#include "boxesgroup.h"
#include "undoredo.h"
#include <QPainter>
#include "keysview.h"

QrealKey *BoundingBox::getKeyAtPos(qreal relX, qreal relY, qreal) {
    if(relY < 0.) return NULL;
    int minViewedFrame = mKeysView->getMinViewedFrame();
    qreal pixelsPerFrame =  mKeysView->getPixelsPerFrame();
    QrealKey *collectionKey = NULL;
    qreal currY = relY;
    if(currY <= BoxesList::getListItemHeight()) {
        collectionKey = mAnimatorsCollection.getKeyAtPos(relX, relY,
                               minViewedFrame,
                               pixelsPerFrame);
    }
    currY -= BoxesList::getListItemHeight();
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

void BoundingBox::selectionChangeTriggered(bool shiftPressed) {
    if(shiftPressed) {
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

void BoundingBox::handleListItemMousePress(qreal boxesListX,
                                           qreal relX, qreal relY,
                                           QMouseEvent *event) {
    if(relX < 0) {
        return;
    }
    if(relY < BoxesList::getListItemHeight()) {
        if(event->button() == Qt::LeftButton) {
            if(relX < BoxesList::getListItemHeight()) {
                setChildrenListItemsVisible(!mBoxListItemDetailsVisible);
            } else if(relX < BoxesList::getListItemHeight()*2) {
                setVisibile(!mVisible);
            } else if(relX < BoxesList::getListItemHeight()*3) {
                setLocked(!mLocked);
            } else {
                if(isVisibleAndUnlocked() && mParent->isCurrentGroup()) {
                    selectionChangeTriggered(isShiftPressed());
                }
            }
        } else if(event->button() == Qt::RightButton) {
            showContextMenu(event->globalPos() );
        }
    } else if(mBoxListItemDetailsVisible) {
        relY -= BoxesList::getListItemHeight();
        foreach(QrealAnimator *animator, mActiveAnimators) {
            qreal animatorHeight = animator->getBoxesListHeight();
            if(relY <= animatorHeight) {
                animator->handleListItemMousePress(boxesListX,
                                                   relX - BoxesList::getListItemChildIndent(),
                                                   relY, event);
                break;
            }
            relY -= animatorHeight;
        }
    }
}

void BoundingBox::getKeysInRect(QRectF selectionRect,
                                QList<QrealKey*> *keysList) {
    qreal rectMargin = (BoxesList::getListItemHeight() - KEY_RECT_SIZE)*0.5;
    if(selectionRect.bottom() < rectMargin) return;
    int minViewedFrame = mKeysView->getMinViewedFrame();
    qreal pixelsPerFrame =  mKeysView->getPixelsPerFrame();
    if(selectionRect.top() < BoxesList::getListItemHeight() - rectMargin) {
        mAnimatorsCollection.getKeysInRect(selectionRect,
                                         minViewedFrame,
                                         pixelsPerFrame,
                                         keysList);
    }
    selectionRect.translate(0., -BoxesList::getListItemHeight());
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

void BoundingBox::addEffect(PixmapEffect *effect) {
    effect->setUpdater(new PixmapEffectUpdater(this));
    effect->incNumberPointers();
    mEffects << effect;

    if(!mEffectsAnimators.hasChildAnimators()) {
        mAnimatorsCollection.addAnimator(&mEffectsAnimators);
        addActiveAnimator(&mEffectsAnimators);
    }
    mEffectsAnimators.addChildAnimator(effect);
}

void BoundingBox::removeEffect(PixmapEffect *effect) {

    mEffects.removeOne(effect);
    removeActiveAnimator(effect);
    mEffectsAnimators.removeChildAnimator(effect);
    if(!mEffectsAnimators.hasChildAnimators()) {
        mAnimatorsCollection.removeAnimator(&mEffectsAnimators);
        removeActiveAnimator(&mEffectsAnimators);
    }
    effect->decNumberPointers();
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
    qreal heightT = BoundingBox::getListItemHeight();
    getKeysInRectFromChildren(selectionRect.translated(0., -heightT),
                              keysList);
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
    return getKeyAtPosFromChildren(relX, relY, BoundingBox::getListItemHeight());
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
        if(mType == BoundingBoxType::TYPE_CANVAS) {
            showContextMenu(event->globalPos() );
            return;
        }
        BoundingBox::handleListItemMousePress(boxesListX,
                                              relX, relY, event);
    } else {
        handleChildListItemMousePress(boxesListX,
                                      (mType == BoundingBoxType::TYPE_CANVAS) ? relX : relX - BoxesList::getListItemHeight(), relY,
                                      heightT, event);
    }
}

void BoundingBox::drawListItem(QPainter *p, qreal drawX, qreal drawY,
                               qreal maxY) {
    Q_UNUSED(maxY);

    drawAnimationBar(p, drawX + BoxesList::getListItemHeight(), drawY);

    p->setPen(Qt::black);
    if(mSelected) {
        p->setBrush(QColor(185, 185, 255));
    } else {
        p->setBrush(QColor(225, 225, 225));
    }
    p->drawRect(QRectF(drawX, drawY,
                       BoxesList::getListItemMaxWidth() - drawX, BoxesList::getListItemHeight()));

    if(mBoxListItemDetailsVisible) {
        p->drawPixmap(drawX, drawY, *BoxesList::HIDE_CHILDREN);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::SHOW_CHILDREN);
    }
    drawX += BoxesList::getListItemHeight();
    if(mVisible) {
        p->drawPixmap(drawX, drawY, *BoxesList::VISIBLE_PIXMAP);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::INVISIBLE_PIXMAP);
    }
    drawX += BoxesList::getListItemHeight();
    if(mLocked) {
        p->drawPixmap(drawX, drawY, *BoxesList::LOCKED_PIXMAP);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::UNLOCKED_PIXMAP);
    }
    drawX += 2*BoxesList::getListItemHeight();
    p->drawText(QRectF(drawX, drawY,
                       BoxesList::getListItemMaxWidth() - drawX - BoxesList::getListItemHeight(),
                       BoxesList::getListItemHeight()),
                mName, QTextOption(Qt::AlignVCenter));

}

void BoundingBox::drawKeysView(QPainter *p, qreal drawY,
                               qreal maxY, qreal pixelsPerFrame,
                               int startFrame, int endFrame) {
    Q_UNUSED(maxY);
    drawKeys(p, pixelsPerFrame, drawY,
             startFrame, endFrame);
}

void BoundingBox::drawAnimationBar(QPainter *p,
                                   qreal drawX, qreal drawY) {
    drawY += BoxesList::getListItemHeight();
    if(mBoxListItemDetailsVisible) {
        foreach(QrealAnimator *animator, mActiveAnimators) {
            animator->drawBoxesList(p, drawX, drawY);
            drawY += animator->getBoxesListHeight();
        }
    }
}

void BoundingBox::drawKeys(QPainter *p,
                           qreal pixelsPerFrame,
                           qreal drawY,
                           int startFrame, int endFrame) {
    mAnimatorsCollection.drawKeys(p,
                                  pixelsPerFrame, 0., drawY, 20.,
                                  startFrame, endFrame);
    drawY += BoxesList::getListItemHeight();
    if(mBoxListItemDetailsVisible) {
        foreach(QrealAnimator *animator, mActiveAnimators) {
            animator->drawKeys(p, pixelsPerFrame, 0., drawY, BoxesList::getListItemHeight(),
                               startFrame, endFrame);
            drawY += animator->getBoxesListHeight();
        }
    }
}

void BoxesGroup::drawChildrenListItems(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal maxY) {
    qreal currentY = drawY;
    foreach (BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(currentY + boxHeight >= 0.) {
            if(currentY > maxY) {
                break;
            }
            box->drawListItem(p, drawX, currentY, maxY);
        }
        currentY += boxHeight;
    }
}

void BoxesGroup::drawChildrenKeysView(QPainter *p,
                              qreal drawY,
                              qreal maxY, qreal pixelsPerFrame,
                              int startFrame, int endFrame) {
    qreal currentY = drawY;
    foreach (BoundingBox *box, mChildren) {
        qreal boxHeight = box->getListItemHeight();
        if(currentY + boxHeight >= 0.) {
            if(currentY > maxY) {
                break;
            }
            box->drawKeysView(p, currentY, maxY,
                              pixelsPerFrame, startFrame, endFrame);
        }
        currentY += boxHeight;
    }
}

void Canvas::drawListItem(QPainter *p, qreal drawX, qreal drawY,
                               qreal maxY) {
    Q_UNUSED(maxY);

    drawAnimationBar(p, drawX + BoxesList::getListItemHeight(), drawY);

    p->setPen(Qt::black);
    if(mSelected) {
        p->setBrush(QColor(185, 185, 255));
    } else {
        p->setBrush(QColor(225, 225, 225));
    }
    p->drawRect(QRectF(drawX, drawY,
                       BoxesList::getListItemMaxWidth() - drawX, BoxesList::getListItemHeight()));

    p->drawText(QRectF(drawX, drawY,
                       BoxesList::getListItemMaxWidth() - drawX - BoxesList::getListItemHeight(),
                       BoxesList::getListItemHeight()),
                "Canvas", QTextOption(Qt::AlignCenter));

    drawY += BoundingBox::getListItemHeight();
    drawChildrenListItems(p, drawX, drawY, maxY);

}

void BoxesGroup::drawListItem(QPainter *p,
                              qreal drawX, qreal drawY,
                              qreal maxY)
{
    BoundingBox::drawListItem(p, drawX, drawY, maxY);
    if(mBoxListItemDetailsVisible) {
        drawX += BoxesList::getListItemChildIndent();
        drawY += BoundingBox::getListItemHeight();
        drawChildrenListItems(p, drawX, drawY, maxY);
    }
}

void BoxesGroup::drawKeysView(QPainter *p,
                              qreal drawY,
                              qreal maxY, qreal pixelsPerFrame,
                              int startFrame, int endFrame)
{
    BoundingBox::drawKeysView(p, drawY, maxY,
                              pixelsPerFrame, startFrame, endFrame);
    if(mBoxListItemDetailsVisible) {
        drawY += BoundingBox::getListItemHeight();
        drawChildrenKeysView(p, drawY, maxY,
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
    if(mBoxListItemDetailsVisible) {
        qreal height = BoxesList::getListItemHeight();
        foreach(QrealAnimator *animator, mActiveAnimators) {
            height += animator->getBoxesListHeight();
        }

        return height;
    } else {
        return BoxesList::getListItemHeight();
    }
}

void BoundingBox::setChildrenListItemsVisible(bool bt)
{
    mBoxListItemDetailsVisible = bt;
    mAnimatorsCollection.setBoxesListDetailVisible(bt);
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
