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
    foreach(Property *property, mChildAnimators) {
        property->decNumberPointers();
    }
    mChildAnimators.clear();
}

int ComplexAnimator::getNumberOfChildren() {
    return mChildAnimators.count();
}

Property *ComplexAnimator::getChildAt(const int &i) {
    return mChildAnimators.at(i);
}
#include <QDebug>
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
void ComplexAnimator::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    foreach(Property *property, mChildAnimators) {
        abstraction->addChildAbstraction(
                    property->SWT_createAbstraction(visiblePartWidget));
    }

}

qreal ComplexAnimator::clampValue(qreal value) {
    return value;//clamp(value, mMinMoveValue, mMaxMoveValue);
}

ComplexKey *ComplexAnimator::getKeyCollectionAtAbsFrame(int frame) {
    return (ComplexKey *) getKeyAtAbsFrame(frame);
}

ComplexKey *ComplexAnimator::getKeyCollectionAtRelFrame(int frame) {
    return (ComplexKey *) getKeyAtRelFrame(frame);
}

void ComplexAnimator::addChildAnimator(Property *childAnimator)
{
    mChildAnimators << childAnimator;
    childAnimator->setUpdater(mUpdater);
    childAnimator->incNumberPointers();
    childAnimator->setParentAnimator(this);

    childAnimator->addAllKeysToComplexAnimator();
    childAnimatorIsRecordingChanged();
    childAnimator->setAbsFrame(mCurrentAbsFrame);
    //updateKeysPath();

    SWT_addChildAbstractionForTargetToAll(childAnimator);

    callUpdater();
}

void ComplexAnimator::moveChildAbove(Property *move,
                                     Property *above) {
    int indexFrom = mChildAnimators.indexOf(move);
    int indexTo = mChildAnimators.indexOf(above);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    moveChildInList(move,
                    indexFrom,
                    indexTo);
}

void ComplexAnimator::moveChildBelow(Property *move,
                                     Property *below) {
    int indexFrom = mChildAnimators.indexOf(move);
    int indexTo = mChildAnimators.indexOf(below);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    moveChildInList(move,
                    indexFrom,
                    indexTo);
}

void ComplexAnimator::moveChildInList(
                                 Property *child,
                                 int from, int to,
                                 bool saveUndoRedo) {
    mChildAnimators.move(from, to);
    SWT_moveChildAbstractionForTargetToInAll(child,
                                             to);
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildAnimatorInListUndoRedo(
                        child, from, to, this) );
    }

    callUpdater();
}

void ComplexAnimator::removeChildAnimator(Property *removeAnimator) {
    removeAnimator->setUpdater(NULL);
    removeAnimator->removeAllKeysFromComplexAnimator();
    mChildAnimators.removeOne(removeAnimator);
    removeAnimator->setParentAnimator(NULL);
    removeAnimator->decNumberPointers();
    childAnimatorIsRecordingChanged();
    updateKeysPath();

    SWT_removeChildAbstractionForTargetFromAll(removeAnimator);

    callUpdater();
}

void ComplexAnimator::swapChildAnimators(Property *animator1,
                                         Property *animator2) {
    int id1 = mChildAnimators.indexOf(animator1);
    int id2 = mChildAnimators.indexOf(animator2);
    mChildAnimators.swap(id1, id2);

    callUpdater();
}

void ComplexAnimator::clearFromGraphView()
{
    QrealAnimator::clearFromGraphView();

    foreach(Property *property, mChildAnimators) {
        property->clearFromGraphView();
    }
}

bool ComplexAnimator::hasChildAnimators()
{
    return !mChildAnimators.isEmpty();
}

void ComplexAnimator::startTransform()
{
    foreach(Property *property, mChildAnimators) {
        property->startTransform();
    }
}

void ComplexAnimator::setTransformed(bool bT) {
    foreach(Property *property, mChildAnimators) {
        property->setTransformed(bT);
    }
}

void ComplexAnimator::drawKey(
                            QPainter *p,
                            QrealKey *key,
                            const qreal &pixelsPerFrame,
                            const qreal &drawY,
                            const int &startFrame) {
    if(key->areAllChildrenSelected()) {
        p->setBrush(Qt::yellow);
    } else {
        p->setBrush(Qt::red);
    }

    if(key->isHovered()) {
        p->save();
        p->setPen(QPen(Qt::black, 1.5));
        p->drawEllipse(
            QRectF(
                QPointF((key->getAbsFrame() - startFrame + 0.5)*
                        pixelsPerFrame - KEY_RECT_SIZE*0.35,
                        drawY + (BOX_HEIGHT -
                                  KEY_RECT_SIZE*0.7)*0.5 ),
                QSize(KEY_RECT_SIZE*0.7, KEY_RECT_SIZE*0.7) ) );
        p->restore();
    } else {
        p->drawEllipse(
            QRectF(
                QPointF((key->getAbsFrame() - startFrame + 0.5)*
                        pixelsPerFrame - KEY_RECT_SIZE*0.35,
                        drawY + (BOX_HEIGHT -
                                  KEY_RECT_SIZE*0.7)*0.5 ),
                QSize(KEY_RECT_SIZE*0.7, KEY_RECT_SIZE*0.7) ) );
    }
}

void ComplexAnimator::changeChildAnimatorZ(const int &oldIndex,
                                           const int &newIndex) {
    mChildAnimators.move(oldIndex, newIndex);

    callUpdater();
}

void ComplexAnimator::setUpdater(AnimatorUpdater *updater)
{
    if(mUpdaterBlocked) return;
    QrealAnimator::setUpdater(updater);

    foreach(Property *property, mChildAnimators) {
        property->setUpdater(updater);
    }
}

void ComplexAnimator::setAbsFrame(int frame)
{
    QrealAnimator::setAbsFrame(frame);

    foreach(Property *property, mChildAnimators) {
        property->setAbsFrame(frame);
    }
}

void ComplexAnimator::retrieveSavedValue()
{
    foreach(Property *property, mChildAnimators) {
        property->retrieveSavedValue();
    }
}

void ComplexAnimator::finishTransform()
{
    foreach(Property *property, mChildAnimators) {
        property->finishTransform();
    }
}

void ComplexAnimator::cancelTransform()
{
    foreach(Property *property, mChildAnimators) {
        property->cancelTransform();
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
    foreach(Property *property, mChildAnimators) {
        property->setRecording(rec);
    }
    setRecordingValue(rec);
}

void ComplexAnimator::childAnimatorIsRecordingChanged()
{
    bool rec = true;
    mChildAnimatorRecording = false;
    foreach(Property *property, mChildAnimators) {
        bool isChildRec = property->isRecording();
        bool isChildDescRec = property->isDescendantRecording();
        if(isChildDescRec) {
            mChildAnimatorRecording = true;
        }
        if(!isChildRec) {
            rec = false;
        }
    }
    setRecordingValue(rec);
}

void ComplexAnimator::addChildQrealKey(QrealKey *key) {
    ComplexKey *collection = getKeyCollectionAtAbsFrame(key->getAbsFrame() );
    if(collection == NULL) {
        collection = new ComplexKey(this);
        collection->setAbsFrame(key->getAbsFrame());
        appendKey(collection);
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::removeChildQrealKey(QrealKey *key) {
    ComplexKey *collection = key->getParentKey();//getKeyCollectionAtAbsFrame(key->getAbsFrame() );
    if(collection == NULL) return;
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

void ComplexKey::copyToContainer(KeysClipboardContainer *container) {
    foreach(QrealKey *key, mKeys) {
        if(key->isSelected()) continue;
        key->copyToContainer(container);
    }
}

QrealKey *ComplexKey::makeQrealKeyDuplicate(QrealAnimator *targetParent) {
    ComplexKey *target = new ComplexKey((ComplexAnimator*)targetParent);
    target->setValue(mValue);
    target->setRelFrame(mRelFrame);
    target->setCtrlsMode(mCtrlsMode);
    target->setStartEnabled(mStartEnabled);
    target->setStartFrame(mStartFrame);
    target->setStartValue(mStartValue);
    target->setEndEnabled(mEndEnabled);
    target->setEndFrame(mEndFrame);
    target->setEndValue(mEndValue);
    //targetParent->appendKey(target);
    foreach(QrealKey *key, mKeys) {
        if(key->isSelected()) continue;
        QrealKey *keyDuplicate = key->makeQrealKeyDuplicate(
                    key->getParentAnimator());
        target->addAnimatorKey(keyDuplicate);
        key->getParentAnimator()->appendKey(keyDuplicate);
    }

    return target;
}

void ComplexKey::setRelFrame(int frame) {
    QrealKey::setRelFrame(frame);

    foreach(QrealKey *key, mKeys) {
        key->setRelFrame(frame);
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

//void ComplexKey::scaleFrameAndUpdateParentAnimator(const int &relativeToFrame,
//                                                   const qreal &scaleFactor) {
//    foreach(QrealKey *key, mKeys) {
//        if(key->isSelected()) continue;
//        key->scaleFrameAndUpdateParentAnimator(relativeToFrame,
//                                               scaleFactor);
//    }
//}

void ComplexKey::cancelFrameTransform() {
    QrealKey::cancelFrameTransform();
//    foreach(QrealKey *key, mKeys) {
//        if(key->isSelected()) continue;
//        key->cancelFrameTransform();
//    }
}

void ComplexKey::addToSelection(QList<QrealKey *> *selectedKeys) {
    foreach(QrealKey *key, mKeys) {
        key->addToSelection(selectedKeys);
    }
}

void ComplexKey::removeFromSelection(QList<QrealKey *> *selectedKeys) {
    foreach(QrealKey *key, mKeys) {
        key->removeFromSelection(selectedKeys);
    }
}

void ComplexKey::startFrameTransform()
{
    QrealKey::startFrameTransform();
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
