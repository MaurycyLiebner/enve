#include "Animators/complexanimator.h"
#include "qrealanimator.h"

ComplexAnimator::ComplexAnimator(const QString &name) : Animator(name) {
    anim_mIsComplexAnimator = true;
}

ComplexAnimator::~ComplexAnimator() {
    anim_removeAllKeys();
}

void ComplexAnimator::ca_prependChildAnimator(Property *childAnimator,
                                              const qsptr<Property> &prependWith) {
    if(prependWith == nullptr) return;
    int id = getChildPropertyIndex(childAnimator);
    if(id == -1) return;
    ca_addChildAnimator(prependWith, id);
}


void ComplexAnimator::ca_replaceChildAnimator(const qsptr<Property>& childAnimator,
                                              const qsptr<Property> &replaceWith) {
    int id = getChildPropertyIndex(childAnimator.get());
    if(id == -1) return;
    ca_removeChildAnimator(childAnimator);
    if(replaceWith == nullptr) return;
    ca_addChildAnimator(replaceWith, id);
}

int ComplexAnimator::ca_getNumberOfChildren() {
    return ca_mChildAnimators.count();
}

Property *ComplexAnimator::ca_getChildAt(const int &i) {
    return ca_mChildAnimators.at(i).data();
}
#include <QDebug>
#include "singlewidgetabstraction.h"
void ComplexAnimator::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction* abstraction,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartWidgetId) {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        auto newAbs = property->SWT_createAbstraction(updateFuncs,
                                                      visiblePartWidgetId);
        abstraction->addChildAbstraction(newAbs);
    }

}

void ComplexAnimator::prp_getFirstAndLastIdenticalRelFrame(
                           int *firstIdentical,
                           int *lastIdentical,
                           const int &relFrame) {
    int fId = INT_MIN;
    int lId = INT_MAX;

    Q_FOREACH(const qsptr<Property> &child, ca_mChildAnimators) {
        if(fId > lId) {
            break;
        }
        int fIdT;
        int lIdT;
        child->prp_getFirstAndLastIdenticalRelFrame(
                    &fIdT, &lIdT,
                    relFrame);
        if(fIdT > fId) {
            fId = fIdT;
        }
        if(lIdT < lId) {
            lId = lIdT;
        }
    }

    if(lId > fId) {
        *firstIdentical = fId;
        *lastIdentical = lId;
    } else {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    }
}


bool ComplexAnimator::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                          const bool &parentSatisfies,
                                          const bool &parentMainTarget) {
    if(hasChildAnimators()) {
        return Animator::SWT_shouldBeVisible(
                    rules,
                    parentSatisfies,
                    parentMainTarget);
    } else {
        return false;
    }
}

bool ComplexAnimator::SWT_isComplexAnimator() { return true; }

ComplexKey *ComplexAnimator::ca_getKeyCollectionAtAbsFrame(const int &frame) {
    return GetAsPtr(anim_getKeyAtAbsFrame(frame), ComplexKey);
}

ComplexKey *ComplexAnimator::ca_getKeyCollectionAtRelFrame(const int &frame) {
    return GetAsPtr(anim_getKeyAtRelFrame(frame), ComplexKey);
}

void ComplexAnimator::ca_addChildAnimator(const qsptr<Property>& childAnimator,
                                          const int &id) {
    ca_mChildAnimators.insert(id, childAnimator);
    childAnimator->setParent(this);
    childAnimator->prp_setUpdater(prp_mUpdater);
    childAnimator->prp_setParentFrameShift(prp_getFrameShift());
    connect(childAnimator.data(), &Property::prp_updateWholeInfluenceRange,
            this, &Property::prp_updateInfluenceRangeAfterChanged);
    connect(childAnimator.data(), &Property::prp_isRecordingChanged,
            this, &ComplexAnimator::ca_childAnimatorIsRecordingChanged);
    connect(childAnimator.data(), &Property::prp_absFrameRangeChanged,
            this, &Property::prp_updateAfterChangedAbsFrameRange);
    connect(childAnimator.data(), &Property::prp_addingKey,
            this, &ComplexAnimator::ca_addDescendantsKey);
    connect(childAnimator.data(), &Property::prp_removingKey,
            this, &ComplexAnimator::ca_removeDescendantsKey);
    connect(childAnimator.data(), &Property::prp_replaceWith,
            this, &ComplexAnimator::ca_replaceChildAnimator);
    connect(childAnimator.data(), &Property::prp_prependWith,
            this, &ComplexAnimator::ca_prependChildAnimator);

    childAnimator->prp_addAllKeysToComplexAnimator(this);
    ca_childAnimatorIsRecordingChanged();
    childAnimator->prp_setAbsFrame(anim_mCurrentAbsFrame);
    //updateKeysPath();

    SWT_addChildAbstractionForTargetToAllAt(childAnimator.get(), id);

    prp_callUpdater();
}

int ComplexAnimator::getChildPropertyIndex(Property *child) {
    for(int i = 0; i < ca_mChildAnimators.count(); i++) {
        if(ca_mChildAnimators.at(i) == child) {
            return i;
        }
    }
    return -1;
}

void ComplexAnimator::ca_updateDescendatKeyFrame(Key* key) {
    Q_FOREACH(const stdsptr<Key> &ckey, anim_mKeys) {
        stdsptr<ComplexKey> complexKey = GetAsSPtr(ckey, ComplexKey);
        if(complexKey->hasKey(key)) {
            complexKey->removeAnimatorKey(key);
            if(complexKey->isEmpty() ) {
                anim_removeKey(complexKey);
            }
            ca_addDescendantsKey(key);
            break;
        }
    }
}

void ComplexAnimator::ca_moveChildAbove(Property *move,
                                        Property *above,
                                        const bool &saveUndoRedo) {
    int indexFrom = getChildPropertyIndex(move);
    int indexTo = getChildPropertyIndex(above);
    if(indexFrom > indexTo) {
        indexTo++;
    }
    ca_moveChildInList(move,
                       indexFrom,
                       indexTo,
                       saveUndoRedo);
}

void ComplexAnimator::ca_moveChildBelow(Property *move,
                                        Property *below) {
    int indexFrom = getChildPropertyIndex(move);
    int indexTo = getChildPropertyIndex(below);
    if(indexFrom < indexTo) {
        indexTo--;
    }
    ca_moveChildInList(move, indexFrom, indexTo);
}

void ComplexAnimator::ca_moveChildInList(
                     Property* child,
                     const int &from, const int &to,
                     const bool &saveUndoRedo) {
    ca_mChildAnimators.move(from, to);
    SWT_moveChildAbstractionForTargetToInAll(child, to);
    if(saveUndoRedo) {
//        addUndoRedo(new MoveChildAnimatorInListUndoRedo(
//                        child, from, to, this) );
    }

    prp_callUpdater();
}

void ComplexAnimator::ca_removeChildAnimator(
        const qsptr<Property>& removeAnimator) {
    removeAnimator->prp_setUpdater(nullptr);
    removeAnimator->prp_removeAllKeysFromComplexAnimator(this);
    disconnect(removeAnimator.get(), nullptr, this, nullptr);

    SWT_removeChildAbstractionForTargetFromAll(removeAnimator.get());

    ca_mChildAnimators.removeAt(getChildPropertyIndex(removeAnimator.get()));
    ca_childAnimatorIsRecordingChanged();

    prp_callUpdater();
}

void ComplexAnimator::ca_removeAllChildAnimators() {
    for(int i = ca_mChildAnimators.count() - 1; i >= 0; i--) {
        ca_removeChildAnimator(ca_mChildAnimators.at(i));
    }
}

Property *ComplexAnimator::ca_getFirstDescendantWithName(const QString &name) {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        if(property->prp_getName() == name) {
            return property.get();
        } else if(property->SWT_isComplexAnimator()) {
            Property* propT = GetAsPtr(property, ComplexAnimator)->
                    ca_getFirstDescendantWithName(name);
            if(propT != nullptr) return propT;
        }
    }
    return nullptr;
}

void ComplexAnimator::ca_swapChildAnimators(Property *animator1,
                                            Property *animator2) {
    int id1 = getChildPropertyIndex(animator1);
    int id2 = getChildPropertyIndex(animator2);
    ca_mChildAnimators.swap(id1, id2);

    prp_callUpdater();
}

void ComplexAnimator::prp_clearFromGraphView() {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_clearFromGraphView();
    }
}

bool ComplexAnimator::hasChildAnimators() {
    return !ca_mChildAnimators.isEmpty();
}

void ComplexAnimator::prp_startTransform() {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_startTransform();
    }
}

void ComplexAnimator::prp_setTransformed(const bool &bT) {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_setTransformed(bT);
    }
}

void ComplexAnimator::anim_drawKey(QPainter *p, Key *key,
                                   const qreal &pixelsPerFrame,
                                   const qreal &drawY,
                                   const int &startFrame,
                                   const int &rowHeight,
                                   const int &keyRectSize) {
    if(key->areAllChildrenSelected()) {
        p->setBrush(Qt::yellow);
    } else {
        p->setBrush(Qt::red);
    }
    if(key->isHovered()) {
        p->setPen(QPen(Qt::black, 1.5));
    } else {
        p->setPen(QPen(Qt::black, 0.5));
    }
    qreal keySize = keyRectSize*0.7;
    p->drawEllipse(
        QRectF(
            QPointF((key->getRelFrame() - startFrame + 0.5)*
                    pixelsPerFrame - keySize*0.5,
                    drawY + (rowHeight -
                              keySize)*0.5 ),
            QSizeF(keySize, keySize) ) );
}

void ComplexAnimator::prp_setParentFrameShift(const int &shift,
                                              ComplexAnimator* parentAnimator) {
    Property::prp_setParentFrameShift(shift, parentAnimator);
    int thisShift = prp_getFrameShift();
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_setParentFrameShift(thisShift, this);
    }
}

void ComplexAnimator::ca_changeChildAnimatorZ(const int &oldIndex,
                                              const int &newIndex) {
    ca_mChildAnimators.move(oldIndex, newIndex);

    prp_callUpdater();
}

void ComplexAnimator::prp_setUpdater(const stdsptr<PropertyUpdater>& updater) {
    if(prp_mUpdaterBlocked) return;
    Animator::prp_setUpdater(updater);

    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_setUpdater(updater);
    }
}

void ComplexAnimator::prp_setAbsFrame(const int &frame) {
    //if(!prp_isDescendantRecording()) return;
    Animator::prp_setAbsFrame(frame);

    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_setAbsFrame(frame);
    }
}

void ComplexAnimator::prp_retrieveSavedValue() {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_retrieveSavedValue();
    }
}

void ComplexAnimator::prp_finishTransform() {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_finishTransform();
    }
}

void ComplexAnimator::prp_cancelTransform() {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_cancelTransform();
    }
}

bool ComplexAnimator::prp_isDescendantRecording() {
    return ca_mChildAnimatorRecording;
}

QString ComplexAnimator::prp_getValueText() {
    return "";
}

void ComplexAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) {
        prp_setRecording(true);
    } else {
        Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
            if(property->SWT_isAnimator()) {
                GetAsPtr(property, Animator)->anim_saveCurrentValueAsKey();
            }
        }
    }
}

void ComplexAnimator::prp_setRecording(const bool &rec) {
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        property->prp_setRecording(rec);
    }
    anim_setRecordingValue(rec);
}

void ComplexAnimator::ca_childAnimatorIsRecordingChanged() {
    bool rec = true;
    bool childRecordingT = false;
    Q_FOREACH(const qsptr<Property> &property, ca_mChildAnimators) {
        bool isChildRec = property->prp_isRecording();
        bool isChildDescRec = property->prp_isDescendantRecording();
        if(isChildDescRec) {
            childRecordingT = true;
        }
        if(!isChildRec) {
            rec = false;
        }
    }
    if(childRecordingT != ca_mChildAnimatorRecording) {
        ca_mChildAnimatorRecording = childRecordingT;
        if(rec != anim_mIsRecording) {
            anim_setRecordingValue(rec);
        } else {
            emit prp_isRecordingChanged();
        }
    } else if(rec != anim_mIsRecording) {
        anim_setRecordingValue(rec);
    }
}

void ComplexAnimator::ca_addDescendantsKey(Key* key) {
    ComplexKey* collection = ca_getKeyCollectionAtAbsFrame(key->getAbsFrame());
    if(collection == nullptr) {
        auto newCollection = SPtrCreate(ComplexKey)(this);
        collection = newCollection.get();
        collection->setAbsFrame(key->getAbsFrame());
        anim_appendKey(newCollection, true, false);
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::ca_removeDescendantsKey(Key* key) {
    ComplexKey* collection =
            ca_getKeyCollectionAtRelFrame(key->getRelFrame());
    if(collection == nullptr) return;
    collection->removeAnimatorKey(key);
    if(collection->isEmpty() ) {
        anim_removeKey(GetAsSPtr(collection, ComplexKey));
    }
}

ComplexKey::ComplexKey(ComplexAnimator* parentAnimator) :
    Key(parentAnimator) {
}

void ComplexKey::addAnimatorKey(Key* key) {
    mKeys << key;
}

void ComplexKey::addOrMergeKey(const stdsptr<Key>& keyAdd) {
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        if(key->getParentAnimator() == keyAdd->getParentAnimator() ) {
            key->mergeWith(keyAdd);
            return;
        }
    }
    addAnimatorKey(keyAdd.get());
}

void ComplexKey::deleteKey() {
    QList<stdptr<Key>> keys = mKeys;
    Q_FOREACH(const stdptr<Key>& key, keys) {
        key->deleteKey();
    }
}

void ComplexKey::removeAnimatorKey(Key* key) {
    mKeys.removeOne(key);
}

bool ComplexKey::isEmpty() {
    return mKeys.isEmpty();
}

void ComplexKey::setRelFrame(const int &frame) {
    Key::setRelFrame(frame);

    int absFrame = mParentAnimator->prp_relFrameToAbsFrame(frame);
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        key->setAbsFrame(absFrame);
    }
}

void ComplexKey::mergeWith(const stdsptr<Key>& key) {
    GetAsPtr(key, ComplexKey)->margeAllKeysToKey(this);
    key->removeFromAnimator();
}

void ComplexKey::margeAllKeysToKey(ComplexKey *target) {
    QList<stdptr<Key>> keys = mKeys;
    Q_FOREACH(const stdptr<Key>& key, keys) {
        removeAnimatorKey(key);
        target->addOrMergeKey(GetAsSPtr(key, Key));
    }
}

bool ComplexKey::isDescendantSelected() {
    if(isSelected()) return true;
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        if(key->isDescendantSelected()) return true;
    }
    return false;
}

//void ComplexKey::scaleFrameAndUpdateParentAnimator(const int &relativeToFrame,
//                                                   const qreal &scaleFactor) {
//    Q_FOREACH(QrealKey *key, mKeys) {
//        if(key->isSelected()) continue;
//        key->scaleFrameAndUpdateParentAnimator(relativeToFrame,
//                                               scaleFactor);
//    }
//}

void ComplexKey::cancelFrameTransform() {
    Key::cancelFrameTransform();
//    Q_FOREACH(QrealKey *key, mKeys) {
//        if(key->isSelected()) continue;
//        key->cancelFrameTransform();
//    }
}

bool ComplexKey::areAllChildrenSelected() {
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        if(key->isSelected() ||
                key->areAllChildrenSelected()) continue;
        return false;
    }

    return true;
}

void ComplexKey::addToSelection(QList<stdptr<Key>> &selectedKeys,
                                QList<qptr<Animator>> &selectedAnimators) {
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        key->addToSelection(selectedKeys, selectedAnimators);
    }
}

bool ComplexKey::hasKey(Key *key) {
    Q_FOREACH(const stdptr<Key>& keyT, mKeys) {
        if(key == keyT) {
            return true;
        }
    }
    return false;
}

bool ComplexKey::differsFromKey(Key *otherKey) {
    ComplexKey* otherComplexKey = GetAsPtr(otherKey, ComplexKey);
    if(getChildKeysCount() == otherComplexKey->getChildKeysCount()) {
        Q_FOREACH(const stdptr<Key>& key, mKeys) {
            if(otherComplexKey->hasSameKey(key)) continue;
            return true;
        }
        return false;
    }
    return true;
}

int ComplexKey::getChildKeysCount() {
    return mKeys.count();
}

bool ComplexKey::hasSameKey(Key *otherKey) {
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        if(key->getParentAnimator() == otherKey->getParentAnimator()) {
            if(key->differsFromKey(otherKey)) return false;
            return true;
        }
    }
    return false;
}

void ComplexKey::removeFromSelection(QList<stdptr<Key>> &selectedKeys,
                                     QList<qptr<Animator>> &selectedAnimators) {
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        key->removeFromSelection(selectedKeys, selectedAnimators);
    }
}

void ComplexKey::startFrameTransform() {
    Key::startFrameTransform();
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        if(key->isSelected()) continue;
        key->startFrameTransform();
    }
}

void ComplexKey::finishFrameTransform() {
    Q_FOREACH(const stdptr<Key>& key, mKeys) {
        if(key->isSelected()) continue;
        key->finishFrameTransform();
    }
}
