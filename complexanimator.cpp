#include "complexanimator.h"
#include "boxeslist.h"

ComplexAnimator::ComplexAnimator() :
    QrealAnimator()
{

}

#include <QDebug>
void ComplexAnimator::drawKeys(QPainter *p, qreal pixelsPerFrame,
                               qreal startX, qreal startY, qreal height,
                               int startFrame, int endFrame,
                               bool detailedView)
{
    QrealAnimator::drawKeys(p, pixelsPerFrame, startX, startY, height,
                            startFrame, endFrame, detailedView);

    if(detailedView) {
        drawChildAnimatorKeys();
    }
}

void ComplexAnimator::drawDiagram(QPainter *p,
                                  qreal pixelsPerFrame, qreal pixelsPerValue,
                                  int startFrame, int endFrame,
                                  bool detailedView) {
//    qreal radius = pixelsPerFrame*0.5;
//    foreach(KeyCollection *key, mKeys) {
//        if(key->frame >= startFrame && key->frame <= endFrame) {
//            p->drawEllipse(
//                    QPointF((key->frame - startFrame)*pixelsPerFrame, centerY),
//                    radius, radius);
//        }
    //    }
}

qreal ComplexAnimator::clampValue(qreal value)
{
    return clamp(value, mMinMoveValue, mMaxMoveValue);
}

ComplexKey *ComplexAnimator::getKeyCollectionAtFrame(int frame) {
    return (ComplexKey *) getKeyAtFrame(frame);
}

void ComplexAnimator::addChildAnimator(QrealAnimator *childAnimator)
{
    mChildAnimators << childAnimator;
    childAnimator->setParentAnimator(this);
}

void ComplexAnimator::removeChildAnimator(QrealAnimator *removeAnimator)
{
    mChildAnimators.removeOne(removeAnimator);
    removeAnimator->setParentAnimator(NULL);
}

void ComplexAnimator::startTransform()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->startTransform();
    }
}

void ComplexAnimator::setConnectedToMainWindow(ConnectedToMainWindow *connected)
{
    QrealAnimator::setConnectedToMainWindow(connected);

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->setConnectedToMainWindow(connected);
    }
}

void ComplexAnimator::setUpdater(AnimatorUpdater *updater)
{
    QrealAnimator::setUpdater(updater);

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->setUpdater(updater);
    }
}

void ComplexAnimator::setFrame(int frame)
{
    QrealAnimator::setFrame(frame);

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->setFrame(frame);
    }
}

void ComplexAnimator::sortKeys()
{
    QrealAnimator::sortKeys();

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->sortKeys();
    }
}

void ComplexAnimator::updateKeysPath()
{
    QrealAnimator::updateKeysPath();

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->updateKeysPath();
    }
}

qreal ComplexAnimator::getBoxesListHeight()
{
    if(mBoxesListDetailVisible) {
        qreal heightT = LIST_ITEM_HEIGHT;
        foreach(QrealAnimator *animator, mChildAnimators) {
            heightT += animator->getBoxesListHeight();
        }
        return heightT;
    } else {
        return LIST_ITEM_HEIGHT;
    }
}

void ComplexAnimator::drawBoxesList(QPainter *p,
                                      qreal drawX, qreal drawY,
                                      qreal pixelsPerFrame,
                                      int startFrame, int endFrame)
{
    QrealAnimator::drawBoxesList(p, drawX, drawY,
                                 pixelsPerFrame, startFrame, endFrame);
    if(mChildAnimatorRecording && !mIsRecording) {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setBrush(Qt::red);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(LIST_ITEM_CHILD_INDENT*0.5 + drawX,
                               LIST_ITEM_HEIGHT*0.5 + drawY),
                       2.5, 2.5);
        p->restore();
    }
    drawX += LIST_ITEM_CHILD_INDENT;
    if(mBoxesListDetailVisible) {
        p->drawPixmap(drawX, drawY,
                      *BoxesList::ANIMATOR_CHILDREN_VISIBLE);
        drawY += LIST_ITEM_HEIGHT;
        foreach(QrealAnimator *animator, mChildAnimators) {
            animator->drawBoxesList(p, drawX, drawY,
                                    pixelsPerFrame,
                                    startFrame, endFrame);
            drawY += animator->getBoxesListHeight();
        }
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::ANIMATOR_CHILDREN_HIDDEN);
    }
}

QrealKey *ComplexAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= LIST_ITEM_HEIGHT) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                   minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        relY -= LIST_ITEM_HEIGHT;
        foreach(QrealAnimator *animator, mChildAnimators) {
            qreal heightT = animator->getBoxesListHeight();
            if(relY <= heightT) {
                return animator->getKeyAtPos(relX, relY,
                                         minViewedFrame, pixelsPerFrame);
            }
            relY -= heightT;
        }
    }
    return NULL;
}

void ComplexAnimator::getKeysInRect(QRectF selectionRect,
                                    int minViewedFrame,
                                    qreal pixelsPerFrame,
                                    QList<QrealKey *> *keysList)
{
    qreal rectMargin = (LIST_ITEM_HEIGHT - KEY_RECT_SIZE)*0.5;
    if(selectionRect.top() <= LIST_ITEM_HEIGHT - rectMargin) {
        QrealAnimator::getKeysInRect(selectionRect, minViewedFrame,
                                     pixelsPerFrame, keysList);
    }
    if(mBoxesListDetailVisible) {
        selectionRect.translate(0., -LIST_ITEM_HEIGHT);
        foreach(QrealAnimator *animator, mChildAnimators) {
            if(selectionRect.bottom() < rectMargin) break;
            qreal heightT = animator->getBoxesListHeight();
            if(selectionRect.top() <= heightT - rectMargin) {
                animator->getKeysInRect(selectionRect, minViewedFrame,
                                        pixelsPerFrame, keysList);
            }
            selectionRect.translate(0., -heightT);
        }
    }
}

void ComplexAnimator::handleListItemMousePress(qreal relX, qreal relY)
{
    if(relY < LIST_ITEM_HEIGHT) {
        QrealAnimator::handleListItemMousePress(relX, relY);
    } else {
        relY -= LIST_ITEM_HEIGHT;
        foreach(QrealAnimator *animator, mChildAnimators) {
            qreal heightT = animator->getBoxesListHeight();
            if(heightT > relY) {
                animator->handleListItemMousePress(
                            relX - LIST_ITEM_CHILD_INDENT,
                            relY);
                break;
            }
            relY -= heightT;
        }
    }
}

void ComplexAnimator::retrieveSavedValue()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->retrieveSavedValue();
    }
}

void ComplexAnimator::finishTransform()
{
    mConnectedToMainWindow->startNewUndoRedoSet();

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->finishTransform();
    }

    mConnectedToMainWindow->finishUndoRedoSet();
}

void ComplexAnimator::cancelTransform()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->cancelTransform();
    }
}

void ComplexAnimator::setRecordingValue(bool rec) {
    mIsRecording = rec;
    if(mParentAnimator != NULL) {
        mParentAnimator->childAnimatorIsRecordingChanged();
    }
}

bool ComplexAnimator::isDescendantRecording()
{
    return mChildAnimatorRecording;
}

QString ComplexAnimator::getValueText()
{
    return "";
}

void ComplexAnimator::setRecording(bool rec)
{
    foreach(QrealAnimator *childAnimator, mChildAnimators) {
        childAnimator->setRecording(rec);
    }
    setRecordingValue(rec);
}

void ComplexAnimator::childAnimatorIsRecordingChanged()
{
    bool rec = true;
    mChildAnimatorRecording = false;
    foreach(QrealAnimator *childAnimator, mChildAnimators) {
        bool isChildRec = childAnimator->isRecording();
        bool isChildDescRec = childAnimator->isDescendantRecording();
        if(isChildDescRec) {
            mChildAnimatorRecording = true;
        }
        if(!isChildRec) {
            rec = false;
        }
    }
    setRecordingValue(rec);
}

void ComplexAnimator::addChildQrealKey(QrealKey *key)
{
    ComplexKey *collection = getKeyCollectionAtFrame(key->getFrame() );
    if(collection == NULL) {
        collection = new ComplexKey(key->getFrame(), this );
        appendKey(collection);
        sortKeys();
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::removeChildQrealKey(QrealKey *key)
{
    ComplexKey *collection = getKeyCollectionAtFrame(key->getFrame() );
    collection->removeAnimatorKey(key);
    if(collection->isEmpty() ) {
        removeKey(collection);
    }
}

ComplexKey::ComplexKey(int frameT, ComplexAnimator *parentAnimator) :
    QrealKey(frameT, parentAnimator, frameT) {
}

void ComplexKey::setStartValue(qreal value) {
    QrealKey::setStartValue(value);

    foreach(QrealKey *key, mKeys) {
        if(key->hasPrevKey() ) {
            qreal prevVal = key->getPrevKeyValue();
            key->setStartValue(value *
                               (key->getValue() - prevVal ) /
                               (getValue() - getPrevKeyValue()) + prevVal );
        }
    }
}

void ComplexKey::setEndValue(qreal value) {
    QrealKey::setEndValue(value);

    foreach(QrealKey *key, mKeys) {
        if(key->hasNextKey() ) {
            key->setEndValue(value *
                             (key->getNextKeyValue() - key->getValue() ) /
                             (getNextKeyValue() - getValue()) + key->getValue() );
        }
    }
}

void ComplexKey::setStartFrame(qreal startFrame) {
    QrealKey::setStartFrame(startFrame);

    foreach(QrealKey *key, mKeys) {
        key->setStartFrame(startFrame);
    }
}

void ComplexKey::setEndFrame(qreal endFrame) {
    QrealKey::setEndFrame(endFrame);

    foreach(QrealKey *key, mKeys) {
        key->setEndFrame(endFrame);
    }
}

void ComplexKey::addAnimatorKey(QrealKey *key) {
    mKeys << key;
    key->setParentKey(this);
    key->incNumberPointers();
}

void ComplexKey::addOrMergeKey(QrealKey *keyAdd) {
    foreach(QrealKey *key, mKeys) {
        if(key->getParentAnimator() == keyAdd->getParentAnimator() ) {
            key->mergeWith(keyAdd);
            return;
        }
    }
    addAnimatorKey(keyAdd);
}

void ComplexKey::deleteKey()
{
    QList<QrealKey*> keys = mKeys;
    foreach(QrealKey *key, keys) {
        key->deleteKey();
    }
}

void ComplexKey::setCtrlsMode(CtrlsMode mode)
{
    QrealKey::setCtrlsMode(mode);

    foreach(QrealKey *key, mKeys) {
        key->setCtrlsMode(mode);
    }
}

void ComplexKey::removeAnimatorKey(QrealKey *key) {
    if(mKeys.removeOne(key) ) {
        key->setParentKey(NULL);
        key->decNumberPointers();
    }
}

bool ComplexKey::isEmpty() {
    return mKeys.isEmpty();
}

void ComplexKey::setValue(qreal) { QrealKey::setValue(mFrame); }

qreal ComplexKey::getValue() { return mFrame; }

void ComplexKey::setFrame(int frame) {
    QrealKey::setFrame(frame);
    QrealKey::setValue(frame);

    foreach(QrealKey *key, mKeys) {
        key->setFrame(frame);
    }
}

void ComplexKey::mergeWith(QrealKey *key) {
    ((ComplexKey*) key)->margeAllKeysToKey(this);
    key->removeFromAnimator();
}

void ComplexKey::margeAllKeysToKey(ComplexKey *target) {
    QList<QrealKey*> keys = mKeys;
    foreach(QrealKey *key, keys) {
        removeAnimatorKey(key);
        target->addOrMergeKey(key); // this might be deleted
    }
}

bool ComplexKey::isDescendantSelected() {
    if(isSelected()) return true;
    foreach(QrealKey *key, mKeys) {
        if(key->isDescendantSelected()) return true;
    }
    return false;
}
