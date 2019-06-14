#include "Animators/complexanimator.h"
#include <QPainter>

ComplexAnimator::ComplexAnimator(const QString &name) : Animator(name) {}

void ComplexAnimator::ca_prependChildAnimator(Property *childAnimator,
                                              const qsptr<Property> &prependWith) {
    if(!prependWith) return;
    const int id = getChildPropertyIndex(childAnimator);
    if(id == -1) return;
    ca_insertChildAnimator(prependWith, id);
}

void ComplexAnimator::ca_replaceChildAnimator(const qsptr<Property>& childAnimator,
                                              const qsptr<Property> &replaceWith) {
    const int id = getChildPropertyIndex(childAnimator.get());
    if(id == -1) return;
    ca_removeChildAnimator(childAnimator);
    if(!replaceWith) return;
    ca_insertChildAnimator(replaceWith, id);
}

int ComplexAnimator::ca_getNumberOfChildren() const {
    return ca_mChildAnimators.count();
}

#include <QDebug>
#include "singlewidgetabstraction.h"
void ComplexAnimator::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction* abstraction,
        const UpdateFuncs &updateFuncs,
        const int visiblePartWidgetId) {
    for(const auto &property : ca_mChildAnimators) {
        auto newAbs = property->SWT_createAbstraction(updateFuncs,
                                                      visiblePartWidgetId);
        abstraction->addChildAbstraction(newAbs);
    }

}

FrameRange ComplexAnimator::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    for(const auto& child : ca_mChildAnimators) {
        const auto childRange = child->prp_getIdenticalRelRange(relFrame);
        range *= childRange;
        if(range.isUnary()) return range;
    }

    return range;
}


bool ComplexAnimator::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                          const bool parentSatisfies,
                                          const bool parentMainTarget) const {
    //if(hasChildAnimators()) {
        return Animator::SWT_shouldBeVisible(rules, parentSatisfies,
                                             parentMainTarget);
    //}
    //return false;
}

bool ComplexAnimator::SWT_isComplexAnimator() const { return true; }

void ComplexAnimator::ca_insertChildAnimator(const qsptr<Property>& childProperty,
                                             const int id) {
    if(mHiddenEmpty && !hasChildAnimators()) {
        SWT_setEnabled(true);
        SWT_setVisible(true);
    }
    if(ca_mChildAnimators.contains(childProperty))
        ca_removeChildAnimator(childProperty);
    ca_mChildAnimators.insert(id, childProperty);
    childProperty->setParent(this);
    childProperty->prp_setInheritedUpdater(prp_mUpdater);
    childProperty->prp_setParentFrameShift(prp_getFrameShift());
    if(childProperty->drawsOnCanvas() ||
       childProperty->SWT_isComplexAnimator()) {
        updateCanvasProps();
    }

    const bool changeInfluence = !(SWT_isBoundingBox() &&
                                   childProperty->SWT_isSingleSound());

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
        childAnimator->anim_setAbsFrame(anim_getCurrentAbsFrame());
    }
    if(changeInfluence){
        connect(childProperty.data(), &Property::prp_absFrameRangeChanged,
                this, &ComplexAnimator::prp_afterChangedAbsRange);
    }
    connect(childProperty.data(), &Property::prp_replaceWith,
            this, &ComplexAnimator::ca_replaceChildAnimator);
    connect(childProperty.data(), &Property::prp_prependWith,
            this, &ComplexAnimator::ca_prependChildAnimator);

    childProperty->SWT_setAncestorDisabled(SWT_isDisabled());
    SWT_addChildAbstractionForTargetToAllAt(childProperty.get(), id);
    if(changeInfluence) {
        const auto childRange = childProperty->prp_absInfluenceRange();
        const auto changedRange = childRange*prp_absInfluenceRange();
        prp_afterChangedAbsRange(changedRange);
    }
}

int ComplexAnimator::getChildPropertyIndex(Property * const child) {
    for(int i = 0; i < ca_mChildAnimators.count(); i++) {
        if(ca_mChildAnimators.at(i) == child) return i;
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
                                         const int to) {
    const int from = getChildPropertyIndex(child);
    if(from == -1) return;
    ca_moveChildInList(child, from, to);
}

void ComplexAnimator::ca_moveChildInList(Property* child,
                                         const int from, const int to) {
    ca_mChildAnimators.move(from, to);
    SWT_moveChildAbstractionForTargetToInAll(child, to);
    prp_afterWholeInfluenceRangeChanged();
}

void ComplexAnimator::ca_removeChildAnimator(
        const qsptr<Property>& removeAnimator) {
    const bool changeInfluence = !(SWT_isBoundingBox() &&
                                   removeAnimator->SWT_isSingleSound());
    const auto childRange = removeAnimator->prp_absInfluenceRange();
    removeAnimator->prp_setInheritedUpdater(nullptr);
    if(removeAnimator->SWT_isAnimator()) {
        const auto aRemove = GetAsPtr(removeAnimator, Animator);
        aRemove->anim_removeAllKeysFromComplexAnimator(this);
    }
    disconnect(removeAnimator.get(), nullptr, this, nullptr);

    SWT_removeChildAbstractionForTargetFromAll(removeAnimator.get());

    removeAnimator->setParent(nullptr);
    ca_mChildAnimators.removeAt(getChildPropertyIndex(removeAnimator.get()));
    if(removeAnimator->drawsOnCanvas() ||
       removeAnimator->SWT_isComplexAnimator()) {
        updateCanvasProps();
    }
    ca_childAnimatorIsRecordingChanged();

    if(changeInfluence) {
        const auto changedRange = childRange*prp_absInfluenceRange();
        prp_afterChangedAbsRange(changedRange);
    }

    if(mHiddenEmpty && !hasChildAnimators()) {
        SWT_setEnabled(false);
        SWT_setVisible(false);
    }
}

void ComplexAnimator::ca_removeAllChildAnimators() {
    for(int i = ca_mChildAnimators.count() - 1; i >= 0; i--)
        ca_removeChildAnimator(qsptr<Property>(ca_mChildAnimators.at(i)));
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

void ComplexAnimator::ca_swapChildAnimators(Property * const animator1,
                                            Property * const animator2) {
    const int id1 = getChildPropertyIndex(animator1);
    const int id2 = getChildPropertyIndex(animator2);
    ca_mChildAnimators.swap(id1, id2);
    prp_afterWholeInfluenceRangeChanged();
}

bool ComplexAnimator::hasChildAnimators() const {
    return !ca_mChildAnimators.isEmpty();
}

void ComplexAnimator::prp_startTransform() {
    for(const auto &property : ca_mChildAnimators)
        property->prp_startTransform();
}

void ComplexAnimator::prp_setTransformed(const bool bT) {
    for(const auto &property : ca_mChildAnimators)
        property->prp_setTransformed(bT);
}

void ComplexAnimator::prp_afterFrameShiftChanged() {
    const int thisShift = prp_getFrameShift();
    for(const auto &property : ca_mChildAnimators)
        property->prp_setParentFrameShift(thisShift, this);
}

void ComplexAnimator::ca_changeChildAnimatorZ(const int oldIndex,
                                              const int newIndex) {
    ca_mChildAnimators.move(oldIndex, newIndex);
    prp_afterWholeInfluenceRangeChanged();
}

void ComplexAnimator::prp_setUpdater(const stdsptr<PropertyUpdater> &updater) {
    Animator::prp_setUpdater(updater);
    for(const auto &property : ca_mChildAnimators)
        property->prp_setInheritedUpdater(updater);
}

void ComplexAnimator::anim_setAbsFrame(const int frame) {
    //if(!anim_isDescendantRecording()) return;
    Animator::anim_setAbsFrame(frame);

    for(const auto &property : ca_mChildAnimators) {
        if(!property->SWT_isAnimator()) continue;
        GetAsPtr(property, Animator)->anim_setAbsFrame(frame);
    }
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
    for(const auto &property : ca_mChildAnimators) {
        if(property->SWT_isAnimator())
            GetAsPtr(property, Animator)->anim_saveCurrentValueAsKey();
    }
}

void ComplexAnimator::anim_setRecording(const bool rec) {
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
    rec = rec && childRecordingT;
    if(childRecordingT != ca_mChildAnimatorRecording) {
        ca_mChildAnimatorRecording = childRecordingT;
        if(rec == anim_isRecording()) emit anim_isRecordingChanged();
    }
    if(rec != anim_isRecording()) {
        anim_setRecordingValue(rec);
    }
}

void ComplexAnimator::ca_addDescendantsKey(Key * const key) {
    auto collection = anim_getKeyAtAbsFrame<ComplexKey>(key->getAbsFrame());
    if(!collection) {
        auto newCollection = SPtrCreate(ComplexKey)(key->getAbsFrame(), this);
        collection = newCollection.get();
        anim_appendKey(newCollection);
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::ca_removeDescendantsKey(Key * const key) {
    const auto collection = anim_getKeyAtRelFrame<ComplexKey>(key->getRelFrame());
    if(!collection) return;
    collection->removeAnimatorKey(key);
    if(collection->isEmpty())
        anim_removeKey(GetAsSPtr(collection, ComplexKey));
}

ComplexKey::ComplexKey(const int absFrame,
                       ComplexAnimator * const parentAnimator) :
    Key(parentAnimator) {
    setAbsFrame(absFrame);
}

void ComplexKey::addAnimatorKey(Key * const key) {
    mKeys << key;
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

//void ComplexKey::setRelFrame(const int frame) {
//    Key::setRelFrame(frame);

//    const int absFrame = mParentAnimator->prp_relFrameToAbsFrame(frame);
//    for(const auto& key : mKeys) {
//        key->setAbsFrame(absFrame);
//    }
//}

void ComplexKey::moveAllKeysTo(ComplexKey * const target) {
    target->mKeys << mKeys;
    mKeys.clear();
}

bool ComplexKey::isDescendantSelected() const {
    if(isSelected()) return true;
    for(const auto& key : mKeys) {
        if(key->isDescendantSelected()) return true;
    }
    return false;
}

//void ComplexKey::scaleFrameAndUpdateParentAnimator(const int relativeToFrame,
//                                                   const qreal scaleFactor) {
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

void ComplexKey::addToSelection(QList<Animator*> &selectedAnimators) {
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

void ComplexKey::removeFromSelection(QList<Animator *> &selectedAnimators) {
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
