#include "complexanimator.h"
#include "boxeslist.h"
#include "mainwindow.h"

ComplexAnimator::ComplexAnimator() :
    QrealAnimator()
{
    mIsComplexAnimator = true;
    mCurrentValue = 0.;
}

ComplexAnimator::~ComplexAnimator()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->decNumberPointers();
    }
    mChildAnimators.clear();
}

#include <QDebug>
void ComplexAnimator::drawKeys(QPainter *p, qreal pixelsPerFrame,
                               qreal startX, qreal startY, qreal height,
                               int startFrame, int endFrame)
{
    QrealAnimator::drawKeys(p, pixelsPerFrame, startX, startY, height,
                            startFrame, endFrame);

    if(mBoxesListDetailVisible) {
        drawChildAnimatorKeys(p, pixelsPerFrame, startX, startY, height,
                              startFrame, endFrame);
    }
}

void ComplexAnimator::drawChildAnimatorKeys(QPainter *p, qreal pixelsPerFrame,
                                            qreal startX, qreal startY, qreal height,
                                            int startFrame, int endFrame)
{
    startY += BoxesList::getListItemHeight();
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->drawKeys(p, pixelsPerFrame, startX, startY, height,
                           startFrame, endFrame);
        startY += animator->getBoxesListHeight();
    }
}

qreal ComplexAnimator::clampValue(qreal value)
{
    return value;//clamp(value, mMinMoveValue, mMaxMoveValue);
}

ComplexKey *ComplexAnimator::getKeyCollectionAtFrame(int frame) {
    return (ComplexKey *) getKeyAtFrame(frame);
}

void ComplexAnimator::addChildAnimator(QrealAnimator *childAnimator)
{
    mChildAnimators << childAnimator;
    childAnimator->incNumberPointers();
    childAnimator->setParentAnimator(this);
    childAnimator->addAllKeysToComplexAnimator();
    childAnimatorIsRecordingChanged();
    childAnimator->setFrame(mCurrentFrame);
    updateKeysPath();
}

void ComplexAnimator::removeChildAnimator(QrealAnimator *removeAnimator)
{
    removeAnimator->removeAllKeysFromComplexAnimator();
    mChildAnimators.removeOne(removeAnimator);
    removeAnimator->setParentAnimator(NULL);
    removeAnimator->decNumberPointers();
    childAnimatorIsRecordingChanged();
    updateKeysPath();
}

void ComplexAnimator::swapChildAnimators(QrealAnimator *animator1,
                                         QrealAnimator *animator2) {
    int id1 = mChildAnimators.indexOf(animator1);
    int id2 = mChildAnimators.indexOf(animator2);
    mChildAnimators.swap(id1, id2);
}

void ComplexAnimator::clearFromGraphView()
{
    QrealAnimator::clearFromGraphView();

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->clearFromGraphView();
    }
}

void ComplexAnimator::startTransform()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->startTransform();
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
        qreal heightT = BoxesList::getListItemHeight();
        foreach(QrealAnimator *animator, mChildAnimators) {
            heightT += animator->getBoxesListHeight();
        }
        return heightT;
    } else {
        return BoxesList::getListItemHeight();
    }
}

void ComplexAnimator::drawBoxesList(QPainter *p,
                                    qreal drawX, qreal drawY)
{
    QrealAnimator::drawBoxesList(p, drawX, drawY);
    if(mChildAnimatorRecording && !mIsRecording) {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setBrush(Qt::red);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(BoxesList::getListItemChildIndent()*0.5 + drawX,
                               BoxesList::getListItemHeight()*0.5 + drawY),
                       2.5, 2.5);
        p->restore();
    }
    drawX += BoxesList::getListItemChildIndent();
    if(mBoxesListDetailVisible) {
        p->drawPixmap(drawX, drawY,
                      *BoxesList::ANIMATOR_CHILDREN_VISIBLE);
        drawY += BoxesList::getListItemHeight();
        foreach(QrealAnimator *animator, mChildAnimators) {
            animator->drawBoxesList(p, drawX, drawY);
            drawY += animator->getBoxesListHeight();
        }
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::ANIMATOR_CHILDREN_HIDDEN);
    }
}

QrealKey *ComplexAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    if(relY <= BoxesList::getListItemHeight()) {
        return QrealAnimator::getKeyAtPos(relX, relY,
                                   minViewedFrame, pixelsPerFrame);
    } else if(mBoxesListDetailVisible) {
        relY -= BoxesList::getListItemHeight();
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
    qreal rectMargin = (BoxesList::getListItemHeight() - KEY_RECT_SIZE)*0.5;
    if(selectionRect.top() <= BoxesList::getListItemHeight() - rectMargin) {
        QrealAnimator::getKeysInRect(selectionRect, minViewedFrame,
                                     pixelsPerFrame, keysList);
    }
    if(mBoxesListDetailVisible) {
        selectionRect.translate(0., -BoxesList::getListItemHeight());
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

void ComplexAnimator::handleListItemMousePress(qreal boxesListX, qreal relX, qreal relY,
                                               QMouseEvent *event)
{
    if(relY < BoxesList::getListItemHeight()) {
        QrealAnimator::handleListItemMousePress(boxesListX, relX, relY, event);
    } else {
        relY -= BoxesList::getListItemHeight();
        foreach(QrealAnimator *animator, mChildAnimators) {
            qreal heightT = animator->getBoxesListHeight();
            if(heightT > relY) {
                animator->handleListItemMousePress(boxesListX,
                            relX - BoxesList::getListItemChildIndent(),
                            relY, event);
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
    startNewUndoRedoSet();

    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->finishTransform();
    }

    finishUndoRedoSet();
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
        collection = new ComplexKey(this);
        collection->setFrame(key->getFrame());
        appendKey(collection);
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::removeChildQrealKey(QrealKey *key)
{
    ComplexKey *collection = getKeyCollectionAtFrame(key->getFrame() );
    collection->removeAnimatorKey(key);
    if(collection->isEmpty() ) {
        removeKey(collection);
        if(mKeys.isEmpty() ) mCurrentValue = 0.;
    }
}

ComplexKey::ComplexKey(ComplexAnimator *parentAnimator) :
    QrealKey(parentAnimator) {
}

void ComplexKey::setStartValue(qreal) {
    QrealKey::setStartValue(getValue());

    foreach(QrealKey *key, mKeys) {
        key->setStartValue(key->getValue());
    }
}

void ComplexKey::setEndValue(qreal) {
    QrealKey::setEndValue(getValue());

    foreach(QrealKey *key, mKeys) {
        key->setStartValue(key->getValue());
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

void ComplexKey::setEndEnabled(bool bT)
{
    QrealKey::setEndEnabled(bT);

    foreach(QrealKey *key, mKeys) {
        key->setEndEnabled(bT);
    }
}

void ComplexKey::setStartEnabled(bool bT)
{
    QrealKey::setStartEnabled(bT);

    foreach(QrealKey *key, mKeys) {
        key->setStartEnabled(bT);
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
