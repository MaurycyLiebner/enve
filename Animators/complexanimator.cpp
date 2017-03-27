#include "Animators/complexanimator.h"
#include "mainwindow.h"
#include "BoxesList/boxsinglewidget.h"

ComplexAnimator::ComplexAnimator() :
    Animator() {
    anim_mIsComplexAnimator = true;
}

ComplexAnimator::~ComplexAnimator() {
    anim_removeAllKeys();
    foreach(Property *property, mChildAnimators) {
        property->decNumberPointers();
    }
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

ComplexKey *ComplexAnimator::getKeyCollectionAtAbsFrame(int frame) {
    return (ComplexKey *) anim_getKeyAtAbsFrame(frame);
}

ComplexKey *ComplexAnimator::getKeyCollectionAtRelFrame(int frame) {
    return (ComplexKey *) anim_getKeyAtRelFrame(frame);
}

void ComplexAnimator::addChildAnimator(Property *childAnimator)
{
    mChildAnimators << childAnimator;
    childAnimator->prp_setUpdater(prp_mUpdater);
    childAnimator->incNumberPointers();
    childAnimator->prp_setParentAnimator(this);

    childAnimator->prp_addAllKeysToComplexAnimator();
    childAnimatorIsRecordingChanged();
    childAnimator->prp_setAbsFrame(anim_mCurrentAbsFrame);
    //updateKeysPath();

    SWT_addChildAbstractionForTargetToAll(childAnimator);

    prp_callUpdater();
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

    prp_callUpdater();
}

void ComplexAnimator::removeChildAnimator(Property *removeAnimator) {
    removeAnimator->prp_setUpdater(NULL);
    removeAnimator->prp_removeAllKeysFromComplexAnimator();
    mChildAnimators.removeOne(removeAnimator);
    removeAnimator->prp_setParentAnimator(NULL);
    removeAnimator->decNumberPointers();
    childAnimatorIsRecordingChanged();

    SWT_removeChildAbstractionForTargetFromAll(removeAnimator);

    prp_callUpdater();
}

void ComplexAnimator::swapChildAnimators(Property *animator1,
                                         Property *animator2) {
    int id1 = mChildAnimators.indexOf(animator1);
    int id2 = mChildAnimators.indexOf(animator2);
    mChildAnimators.swap(id1, id2);

    prp_callUpdater();
}

void ComplexAnimator::prp_clearFromGraphView() {
    foreach(Property *property, mChildAnimators) {
        property->prp_clearFromGraphView();
    }
}

bool ComplexAnimator::hasChildAnimators()
{
    return !mChildAnimators.isEmpty();
}

void ComplexAnimator::prp_startTransform()
{
    foreach(Property *property, mChildAnimators) {
        property->prp_startTransform();
    }
}

void ComplexAnimator::prp_setTransformed(bool bT) {
    foreach(Property *property, mChildAnimators) {
        property->prp_setTransformed(bT);
    }
}

void ComplexAnimator::anim_drawKey(
                            QPainter *p,
                            Key *key,
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

    prp_callUpdater();
}

void ComplexAnimator::prp_setUpdater(AnimatorUpdater *updater) {
    if(prp_mUpdaterBlocked) return;
    Animator::prp_setUpdater(updater);

    foreach(Property *property, mChildAnimators) {
        property->prp_setUpdater(updater);
    }
}

void ComplexAnimator::prp_setAbsFrame(int frame) {
    Animator::prp_setAbsFrame(frame);

    foreach(Property *property, mChildAnimators) {
        property->prp_setAbsFrame(frame);
    }
}

void ComplexAnimator::prp_retrieveSavedValue() {
    foreach(Property *property, mChildAnimators) {
        property->prp_retrieveSavedValue();
    }
}

void ComplexAnimator::prp_finishTransform() {
    foreach(Property *property, mChildAnimators) {
        property->prp_finishTransform();
    }
}

void ComplexAnimator::prp_cancelTransform() {
    foreach(Property *property, mChildAnimators) {
        property->prp_cancelTransform();
    }
}

void ComplexAnimator::setRecordingValue(bool rec) {
    anim_mIsRecording = rec;
    if(prp_mParentAnimator != NULL) {
        prp_mParentAnimator->childAnimatorIsRecordingChanged();
    }
}

bool ComplexAnimator::prp_isDescendantRecording()
{
    return mChildAnimatorRecording;
}

QString ComplexAnimator::prp_getValueText()
{
    return "";
}

void ComplexAnimator::prp_setRecording(bool rec)
{
    foreach(Property *property, mChildAnimators) {
        property->prp_setRecording(rec);
    }
    setRecordingValue(rec);
}

void ComplexAnimator::childAnimatorIsRecordingChanged()
{
    bool rec = true;
    mChildAnimatorRecording = false;
    foreach(Property *property, mChildAnimators) {
        bool isChildRec = property->prp_isRecording();
        bool isChildDescRec = property->prp_isDescendantRecording();
        if(isChildDescRec) {
            mChildAnimatorRecording = true;
        }
        if(!isChildRec) {
            rec = false;
        }
    }
    setRecordingValue(rec);
}

void ComplexAnimator::ca_addDescendantsKey(Key *key) {
    ComplexKey *collection = getKeyCollectionAtAbsFrame(key->getAbsFrame() );
    if(collection == NULL) {
        collection = new ComplexKey(this);
        collection->setAbsFrame(key->getAbsFrame());
        anim_appendKey(collection);
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::ca_removeDescendantsKey(Key *key) {
    ComplexKey *collection = key->getParentKey();//getKeyCollectionAtAbsFrame(key->getAbsFrame() );
    if(collection == NULL) return;
    collection->removeAnimatorKey(key);
    if(collection->isEmpty() ) {
        anim_removeKey(collection);
    }
}

ComplexKey::ComplexKey(ComplexAnimator *parentAnimator) :
    Key(parentAnimator) {
}

void ComplexKey::addAnimatorKey(Key *key) {
    mKeys << key;
    key->setParentKey(this);
    key->incNumberPointers();
}

void ComplexKey::addOrMergeKey(Key *keyAdd) {
    foreach(Key *key, mKeys) {
        if(key->getParentAnimator() == keyAdd->getParentAnimator() ) {
            key->mergeWith(keyAdd);
            return;
        }
    }
    addAnimatorKey(keyAdd);
}

void ComplexKey::deleteKey() {
    QList<Key*> keys = mKeys;
    foreach(Key *key, keys) {
        key->deleteKey();
    }
}

void ComplexKey::removeAnimatorKey(Key *key) {
    if(mKeys.removeOne(key) ) {
        //key->setParentKey(NULL);
        key->decNumberPointers();
    }
}

bool ComplexKey::isEmpty() {
    return mKeys.isEmpty();
}

void ComplexKey::copyToContainer(KeysClipboardContainer *container) {
    foreach(Key *key, mKeys) {
        if(key->isSelected()) continue;
        key->copyToContainer(container);
    }
}

//QrealKey *ComplexKey::makeQrealKeyDuplicate(QrealAnimator *targetParent) {
//    ComplexKey *target = new ComplexKey((ComplexAnimator*)targetParent);
//    target->setValue(mValue);
//    target->setRelFrame(mRelFrame);
//    target->setCtrlsMode(mCtrlsMode);
//    target->setStartEnabled(mStartEnabled);
//    target->setStartFrame(mStartFrame);
//    target->setStartValue(mStartValue);
//    target->setEndEnabled(mEndEnabled);
//    target->setEndFrame(mEndFrame);
//    target->setEndValue(mEndValue);
//    //targetParent->appendKey(target);
//    foreach(QrealKey *key, mKeys) {
//        if(key->isSelected()) continue;
//        QrealKey *keyDuplicate = key->makeQrealKeyDuplicate(
//                    key->getParentAnimator());
//        target->addAnimatorKey(keyDuplicate);
//        key->getParentAnimator()->anim_appendKey(keyDuplicate);
//    }

//    return target;
//}

void ComplexKey::setRelFrame(int frame) {
    Key::setRelFrame(frame);

    foreach(Key *key, mKeys) {
        key->setRelFrame(frame);
    }
}

void ComplexKey::mergeWith(Key *key) {
    ((ComplexKey*) key)->margeAllKeysToKey(this);
    key->removeFromAnimator();
}

void ComplexKey::margeAllKeysToKey(ComplexKey *target) {
    QList<Key*> keys = mKeys;
    foreach(Key *key, keys) {
        removeAnimatorKey(key);
        target->addOrMergeKey(key); // this might be deleted
    }
}

bool ComplexKey::isDescendantSelected() {
    if(isSelected()) return true;
    foreach(Key *key, mKeys) {
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
    Key::cancelFrameTransform();
//    foreach(QrealKey *key, mKeys) {
//        if(key->isSelected()) continue;
//        key->cancelFrameTransform();
//    }
}

void ComplexKey::addToSelection(QList<Key *> *selectedKeys) {
    foreach(Key *key, mKeys) {
        key->addToSelection(selectedKeys);
    }
}

void ComplexKey::removeFromSelection(QList<Key *> *selectedKeys) {
    foreach(Key *key, mKeys) {
        key->removeFromSelection(selectedKeys);
    }
}

void ComplexKey::startFrameTransform() {
    Key::startFrameTransform();
    foreach(Key *key, mKeys) {
        if(key->isSelected()) continue;
        key->startFrameTransform();
    }
}

void ComplexKey::finishFrameTransform() {
    foreach(Key *key, mKeys) {
        if(key->isSelected()) continue;
        key->finishFrameTransform();
    }
}
