#include "Animators/complexanimator.h"
#include <QPainter>

ComplexAnimator::ComplexAnimator(const QString &name) : Animator(name) {}

ComplexAnimator::~ComplexAnimator() {
    anim_removeAllKeys();
}

void ComplexAnimator::ca_prependChildAnimator(Property *childAnimator,
                                              const qsptr<Property> &prependWith) {
    if(!prependWith) return;
    const int id = getChildPropertyIndex(childAnimator);
    if(id == -1) return;
    ca_addChildAnimator(prependWith, id);
}


void ComplexAnimator::ca_replaceChildAnimator(const qsptr<Property>& childAnimator,
                                              const qsptr<Property> &replaceWith) {
    const int id = getChildPropertyIndex(childAnimator.get());
    if(id == -1) return;
    ca_removeChildAnimator(childAnimator);
    if(!replaceWith) return;
    ca_addChildAnimator(replaceWith, id);
}

int ComplexAnimator::ca_getNumberOfChildren() {
    return ca_mChildAnimators.count();
}

Property *ComplexAnimator::ca_getChildAt(const int &i) {
    Q_ASSERT(i >= 0 && i < ca_mChildAnimators.count());
    return ca_mChildAnimators.at(i).data();
}

#include <QDebug>
#include "singlewidgetabstraction.h"
void ComplexAnimator::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction* abstraction,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartWidgetId) {
    for(const auto &property : ca_mChildAnimators) {
        auto newAbs = property->SWT_createAbstraction(updateFuncs,
                                                      visiblePartWidgetId);
        abstraction->addChildAbstraction(newAbs);
    }

}

FrameRange ComplexAnimator::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    for(const auto& child : ca_mChildAnimators) {
        auto childRange = child->prp_getIdenticalRelFrameRange(relFrame);
        range *= childRange;
        if(range.isUnary()) return range;
    }

    return range;
}


bool ComplexAnimator::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                          const bool &parentSatisfies,
                                          const bool &parentMainTarget) const {
    if(hasChildAnimators()) {
        return Animator::SWT_shouldBeVisible(rules, parentSatisfies,
                                             parentMainTarget);
    }
    return false;
}

bool ComplexAnimator::SWT_isComplexAnimator() const { return true; }

ComplexKey *ComplexAnimator::ca_getKeyCollectionAtAbsFrame(const int &frame) {
    return GetAsPtr(anim_getKeyAtAbsFrame(frame), ComplexKey);
}

ComplexKey *ComplexAnimator::ca_getKeyCollectionAtRelFrame(const int &frame) {
    return GetAsPtr(anim_getKeyAtRelFrame(frame), ComplexKey);
}

void ComplexAnimator::ca_addChildAnimator(const qsptr<Property>& childProperty,
                                          const int &id) {
    ca_mChildAnimators.insert(id, childProperty);
    childProperty->setParent(this);
    childProperty->prp_setInheritedUpdater(prp_mUpdater);
    childProperty->prp_setParentFrameShift(prp_getFrameShift());
    connect(childProperty.data(), &Property::prp_updateWholeInfluenceRange,
            this, &Property::prp_updateInfluenceRangeAfterChanged);
    if(childProperty->SWT_isAnimator()) {
        const auto childAnimator = GetAsPtr(childProperty, Animator);
        connect(childAnimator, &Animator::anim_isRecordingChanged,
                this, &ComplexAnimator::ca_childAnimatorIsRecordingChanged);
        connect(childAnimator, &Animator::prp_addingKey,
                this, &ComplexAnimator::ca_addDescendantsKey);
        connect(childAnimator, &Animator::prp_removingKey,
                this, &ComplexAnimator::ca_removeDescendantsKey);
        childAnimator->anim_addAllKeysToComplexAnimator(this);
        ca_childAnimatorIsRecordingChanged();
        childAnimator->anim_setAbsFrame(anim_mCurrentAbsFrame);
    }
    connect(childProperty.data(), &Property::prp_absFrameRangeChanged,
            this, &ComplexAnimator::prp_updateAfterChangedAbsFrameRange);
    connect(childProperty.data(), &Property::prp_replaceWith,
            this, &ComplexAnimator::ca_replaceChildAnimator);
    connect(childProperty.data(), &Property::prp_prependWith,
            this, &ComplexAnimator::ca_prependChildAnimator);

    //updateKeysPath();

    SWT_addChildAbstractionForTargetToAllAt(childProperty.get(), id);
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
    for(const auto& ckey : anim_mKeys) {
        const auto complexKey = GetAsSPtr(ckey, ComplexKey);
        if(complexKey->hasKey(key)) {
            complexKey->removeAnimatorKey(key);
            if(complexKey->isEmpty()) anim_removeKey(complexKey);
            ca_addDescendantsKey(key);
            break;
        }
    }
}

void ComplexAnimator::ca_moveChildAbove(Property *move, Property *above) {
    const int indexFrom = getChildPropertyIndex(move);
    int indexTo = getChildPropertyIndex(above);
    if(indexFrom > indexTo) indexTo++;
    ca_moveChildInList(move, indexFrom, indexTo);
}

void ComplexAnimator::ca_moveChildBelow(Property *move, Property *below) {
    const int indexFrom = getChildPropertyIndex(move);
    int indexTo = getChildPropertyIndex(below);
    if(indexFrom < indexTo) indexTo--;
    ca_moveChildInList(move, indexFrom, indexTo);
}

void ComplexAnimator::ca_moveChildInList(Property* child,
                                         const int &from, const int &to) {
    ca_mChildAnimators.move(from, to);
    SWT_moveChildAbstractionForTargetToInAll(child, to);
}

void ComplexAnimator::ca_removeChildAnimator(
        const qsptr<Property>& removeAnimator) {
    removeAnimator->prp_setInheritedUpdater(nullptr);
    if(removeAnimator->SWT_isAnimator()) {
        const auto aRemove = GetAsPtr(removeAnimator, Animator);
        aRemove->anim_removeAllKeysFromComplexAnimator(this);
    }
    disconnect(removeAnimator.get(), nullptr, this, nullptr);

    SWT_removeChildAbstractionForTargetFromAll(removeAnimator.get());

    ca_mChildAnimators.removeAt(getChildPropertyIndex(removeAnimator.get()));
    ca_childAnimatorIsRecordingChanged();
}

void ComplexAnimator::ca_removeAllChildAnimators() {
    for(int i = ca_mChildAnimators.count() - 1; i >= 0; i--)
        ca_removeChildAnimator(ca_mChildAnimators.at(i));
}

Property *ComplexAnimator::ca_getFirstDescendantWithName(const QString &name) {
    for(const auto &property : ca_mChildAnimators) {
        if(property->prp_getName() == name) {
            return property.get();
        } else if(property->SWT_isComplexAnimator()) {
            const auto propT = GetAsPtr(property, ComplexAnimator)->
                    ca_getFirstDescendantWithName(name);
            if(propT) return propT;
        }
    }
    return nullptr;
}

void ComplexAnimator::ca_swapChildAnimators(Property *animator1,
                                            Property *animator2) {
    const int id1 = getChildPropertyIndex(animator1);
    const int id2 = getChildPropertyIndex(animator2);
    ca_mChildAnimators.swap(id1, id2);
}

bool ComplexAnimator::hasChildAnimators() const {
    return !ca_mChildAnimators.isEmpty();
}

void ComplexAnimator::prp_startTransform() {
    for(const auto &property : ca_mChildAnimators)
        property->prp_startTransform();
}

void ComplexAnimator::prp_setTransformed(const bool &bT) {
    for(const auto &property : ca_mChildAnimators)
        property->prp_setTransformed(bT);
}

void ComplexAnimator::anim_drawKey(QPainter *p, Key *key,
                                   const qreal &pixelsPerFrame,
                                   const qreal &drawY,
                                   const int &startFrame,
                                   const int &rowHeight,
                                   const int &keyRectSize) {
    if(key->isSelected()) p->setBrush(Qt::yellow);
    else p->setBrush(Qt::red);
    if(key->isHovered()) p->setPen(QPen(Qt::black, 1.5));
    else p->setPen(QPen(Qt::black, 0.5));
    const qreal keySize = keyRectSize*0.7;
    const int frameRelToStart = key->getRelFrame() - startFrame;
    const QPointF keyCenter((frameRelToStart + 0.5)*pixelsPerFrame,
                            drawY + 0.5*rowHeight);
    p->drawEllipse(keyCenter, keySize, keySize);
}

void ComplexAnimator::prp_setParentFrameShift(const int &shift,
                                              ComplexAnimator* parentAnimator) {
    Property::prp_setParentFrameShift(shift, parentAnimator);
    const int thisShift = prp_getFrameShift();
    for(const auto &property : ca_mChildAnimators)
        property->prp_setParentFrameShift(thisShift, this);
}

void ComplexAnimator::ca_changeChildAnimatorZ(const int &oldIndex,
                                              const int &newIndex) {
    ca_mChildAnimators.move(oldIndex, newIndex);
}

void ComplexAnimator::prp_setUpdater(const stdsptr<PropertyUpdater> &updater) {
    Animator::prp_setUpdater(updater);
    for(const auto &property : ca_mChildAnimators)
        property->prp_setInheritedUpdater(updater);
}

void ComplexAnimator::anim_setAbsFrame(const int &frame) {
    //if(!anim_isDescendantRecording()) return;
    Animator::anim_setAbsFrame(frame);

    for(const auto &property : ca_mChildAnimators) {
        if(!property->SWT_isAnimator()) continue;
        GetAsPtr(property, Animator)->anim_setAbsFrame(frame);
    }
}

void ComplexAnimator::prp_retrieveSavedValue() {
    for(const auto &property : ca_mChildAnimators)
        property->prp_retrieveSavedValue();
}

void ComplexAnimator::prp_finishTransform() {
    for(const auto &property : ca_mChildAnimators)
        property->prp_finishTransform();
}

void ComplexAnimator::prp_cancelTransform() {
    for(const auto &property : ca_mChildAnimators)
        property->prp_cancelTransform();
}

bool ComplexAnimator::anim_isDescendantRecording() const {
    return ca_mChildAnimatorRecording;
}

QString ComplexAnimator::prp_getValueText() {
    return "";
}

void ComplexAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) {
        anim_setRecording(true);
    } else {
        for(const auto &property : ca_mChildAnimators) {
            if(property->SWT_isAnimator())
                GetAsPtr(property, Animator)->anim_saveCurrentValueAsKey();
        }
    }
}

void ComplexAnimator::anim_setRecording(const bool &rec) {
    for(const auto &property : ca_mChildAnimators) {
        if(!property->SWT_isAnimator()) continue;
        GetAsPtr(property, Animator)->anim_setRecording(rec);
    }
    anim_setRecordingValue(rec);
}

void ComplexAnimator::ca_childAnimatorIsRecordingChanged() {
    bool rec = true;
    bool childRecordingT = false;
    for(const auto &property : ca_mChildAnimators) {
        if(!property->SWT_isAnimator()) continue;
        const auto animator = GetAsPtr(property, Animator);
        const bool isChildRec = animator->anim_isRecording();
        const bool isChildDescRec = animator->anim_isDescendantRecording();
        if(isChildDescRec) childRecordingT = true;
        if(!isChildRec) rec = false;
    }
    if(childRecordingT != ca_mChildAnimatorRecording) {
        ca_mChildAnimatorRecording = childRecordingT;
        if(rec != anim_mIsRecording) anim_setRecordingValue(rec);
        else emit anim_isRecordingChanged();
    } else if(rec != anim_mIsRecording) {
        anim_setRecordingValue(rec);
    }
}

void ComplexAnimator::ca_addDescendantsKey(Key * const key) {
    auto collection = ca_getKeyCollectionAtAbsFrame(key->getAbsFrame());
    if(!collection) {
        auto newCollection = SPtrCreate(ComplexKey)(key->getAbsFrame(), this);
        collection = newCollection.get();
        anim_appendKey(newCollection);
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::ca_removeDescendantsKey(Key * const key) {
    const auto collection = ca_getKeyCollectionAtRelFrame(key->getRelFrame());
    if(!collection) return;
    collection->removeAnimatorKey(key);
    if(collection->isEmpty())
        anim_removeKey(GetAsSPtr(collection, ComplexKey));
}

ComplexKey::ComplexKey(const int &absFrame,
                       ComplexAnimator * const parentAnimator) :
    Key(parentAnimator) {
    setAbsFrame(absFrame);
}

void ComplexKey::addAnimatorKey(Key * const key) {
    mKeys << key;
}

void ComplexKey::addOrMergeKey(const stdsptr<Key>& keyAdd) {
    for(const auto& key : mKeys) {
        if(key->getParentAnimator() == keyAdd->getParentAnimator() ) {
            key->mergeWith(keyAdd);
            return;
        }
    }
    addAnimatorKey(keyAdd.get());
}

void ComplexKey::deleteKey() {
    const auto keys = mKeys;
    for(const auto& key : keys) key->deleteKey();
}

void ComplexKey::removeAnimatorKey(Key * const key) {
    mKeys.removeOne(key);
}

bool ComplexKey::isEmpty() const {
    return mKeys.isEmpty();
}

//void ComplexKey::setRelFrame(const int &frame) {
//    Key::setRelFrame(frame);

//    const int absFrame = mParentAnimator->prp_relFrameToAbsFrame(frame);
//    for(const auto& key : mKeys) {
//        key->setAbsFrame(absFrame);
//    }
//}

void ComplexKey::mergeWith(const stdsptr<Key>& key) {
    GetAsPtr(key, ComplexKey)->margeAllKeysToKey(this);
    key->removeFromAnimator();
}

void ComplexKey::margeAllKeysToKey(ComplexKey * const target) {
    const auto keys = mKeys;
    for(const auto& key : keys) {
        removeAnimatorKey(key);
        target->addOrMergeKey(GetAsSPtr(key, Key));
    }
}

bool ComplexKey::isDescendantSelected() const {
    if(isSelected()) return true;
    for(const auto& key : mKeys) {
        if(key->isDescendantSelected()) return true;
    }
    return false;
}

//void ComplexKey::scaleFrameAndUpdateParentAnimator(const int &relativeToFrame,
//                                                   const qreal &scaleFactor) {
//    for(QrealKey *key : mKeys) {
//        if(key->isSelected()) continue;
//        key->scaleFrameAndUpdateParentAnimator(relativeToFrame,
//                                               scaleFactor);
//    }
//}

void ComplexKey::cancelFrameTransform() {
    Key::cancelFrameTransform();
//    for(QrealKey *key : mKeys) {
//        if(key->isSelected()) continue;
//        key->cancelFrameTransform();
//    }
}

bool ComplexKey::isSelected() const {
    for(const auto& key : mKeys) {
        if(key->isSelected()) continue;
        return false;
    }

    return true;
}

void ComplexKey::addToSelection(QList<qptr<Animator>> &selectedAnimators) {
    for(const auto& key : mKeys) key->addToSelection(selectedAnimators);
}

bool ComplexKey::hasKey(Key *key) const {
    for(const auto& keyT : mKeys) {
        if(key == keyT) return true;
    }
    return false;
}

bool ComplexKey::differsFromKey(Key *otherKey) const {
    const auto otherComplexKey = GetAsPtr(otherKey, ComplexKey);
    if(getChildKeysCount() == otherComplexKey->getChildKeysCount()) {
        for(const auto& key : mKeys) {
            if(otherComplexKey->hasSameKey(key)) continue;
            return true;
        }
        return false;
    }
    return true;
}

void ComplexKey::removeFromSelection(QList<qptr<Animator>> &selectedAnimators) {
    for(const auto& key : mKeys)
        key->removeFromSelection(selectedAnimators);
}

int ComplexKey::getChildKeysCount() const {
    return mKeys.count();
}

bool ComplexKey::hasSameKey(Key *otherKey) const {
    for(const auto& key : mKeys) {
        if(key->getParentAnimator() == otherKey->getParentAnimator()) {
            if(key->differsFromKey(otherKey)) return false;
            return true;
        }
    }
    return false;
}

void ComplexKey::startFrameTransform() {
    Key::startFrameTransform();
    for(const auto& key : mKeys) {
        if(key->isSelected()) continue;
        key->startFrameTransform();
    }
}

void ComplexKey::finishFrameTransform() {
    for(const auto& key : mKeys) {
        if(key->isSelected()) continue;
        key->finishFrameTransform();
    }
}
