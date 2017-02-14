#include "Animators/complexanimator.h"
#include "mainwindow.h"
#include "BoxesList/boxsinglewidget.h"

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

int ComplexAnimator::getNumberOfChildren() {
    return mChildAnimators.count();
}

QrealAnimator *ComplexAnimator::getChildAt(const int &i) {
    return mChildAnimators.at(i);
}
#include <QDebug>
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void ComplexAnimator::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    foreach(QrealAnimator *animator, mChildAnimators) {
        abstraction->addChildAbstraction(
                    animator->SWT_createAbstraction(visiblePartWidget));
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

    emit childAnimatorAdded(childAnimator);

    SWT_addChildAbstractionForTargetToAll(childAnimator);
}

void ComplexAnimator::removeChildAnimator(QrealAnimator *removeAnimator)
{
    removeAnimator->removeAllKeysFromComplexAnimator();
    mChildAnimators.removeOne(removeAnimator);
    removeAnimator->setParentAnimator(NULL);
    removeAnimator->decNumberPointers();
    childAnimatorIsRecordingChanged();
    updateKeysPath();

    emit childAnimatorRemoved(removeAnimator);

    SWT_removeChildAbstractionForTargetFromAll(removeAnimator);
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

bool ComplexAnimator::hasChildAnimators()
{
    return !mChildAnimators.isEmpty();
}

void ComplexAnimator::startTransform()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->startTransform();
    }
}

void ComplexAnimator::setTransformed(bool bT) {
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->setTransformed(bT);
    }
}

void ComplexAnimator::drawKey(
                            QPainter *p,
                            QrealKey *key,
                            const qreal &pixelsPerFrame,
                            const qreal &drawY,
                            const int &startFrame) {
    if(key->isSelected() ) {
        p->setBrush(Qt::yellow);
    } else {
        p->setBrush(Qt::red);
    }
    p->drawEllipse(
        QRectF(
            QPointF((key->getFrame() - startFrame + 0.5)*
                    pixelsPerFrame - KEY_RECT_SIZE*0.35,
                    drawY + (BOX_HEIGHT -
                              KEY_RECT_SIZE*0.7)*0.5 ),
            QSize(KEY_RECT_SIZE*0.7, KEY_RECT_SIZE*0.7) ) );
}

void ComplexAnimator::changeChildAnimatorZ(const int &oldIndex,
                                           const int &newIndex) {
    mChildAnimators.move(oldIndex, newIndex);

    callUpdater();

    emit childAnimatorZChanged(oldIndex, newIndex);
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

void ComplexAnimator::retrieveSavedValue()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->retrieveSavedValue();
    }
}

void ComplexAnimator::finishTransform()
{
    foreach(QrealAnimator *animator, mChildAnimators) {
        animator->finishTransform();
    }
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
        //key->setParentKey(NULL);
        key->decNumberPointers();
    }
}

bool ComplexKey::isEmpty() {
    return mKeys.isEmpty();
}

void ComplexKey::setValue(qreal, bool) { QrealKey::setValue(mFrame, false); }

qreal ComplexKey::getValue() { return mFrame; }

void ComplexKey::setFrame(int frame) {
    QrealKey::setFrame(frame);
    ComplexKey::setValue(frame);

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

void ComplexKey::startFrameTransform()
{
    foreach(QrealKey *key, mKeys) {
        if(key->isSelected()) continue;
        key->startFrameTransform();
    }
}

void ComplexKey::finishFrameTransform()
{
    foreach(QrealKey *key, mKeys) {
        if(key->isSelected()) continue;
        key->finishFrameTransform();
    }
}
