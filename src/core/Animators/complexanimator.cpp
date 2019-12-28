// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "complexanimator.h"
#include "complexkey.h"
#include <QPainter>

ComplexAnimator::ComplexAnimator(const QString &name) :
    Animator(name) {
    SWT_setEnabled(false);
}

void ComplexAnimator::ca_prependChild(Property *child,
                                      const qsptr<Property> &prependWith) {
    if(!prependWith) return;
    const int id = ca_getChildPropertyIndex(child);
    if(id == -1) return;
    ca_insertChild(prependWith, id);
}

void ComplexAnimator::ca_replaceChild(const qsptr<Property>& child,
                                      const qsptr<Property> &replaceWith) {
    const int id = ca_getChildPropertyIndex(child.get());
    if(id == -1) return;
    ca_removeChild(child);
    if(!replaceWith) return;
    ca_insertChild(replaceWith, id);
}

void ComplexAnimator::ca_setHiddenWhenEmpty(const bool hidden) {
    if(ca_mHiddenEmpty == hidden) return;
    ca_mHiddenEmpty = hidden;
    SWT_setVisible(ca_hasChildren() || !hidden);
}

void ComplexAnimator::ca_setDisabledWhenEmpty(const bool disabled) {
    if(ca_mDisabledEmpty == disabled) return;
    ca_mDisabledEmpty = disabled;
    SWT_setEnabled(!ca_mDisabledEmpty || ca_hasChildren());
}

int ComplexAnimator::ca_getNumberOfChildren() const {
    return ca_mChildren.count();
}

#include <QDebug>
#include "singlewidgetabstraction.h"
void ComplexAnimator::SWT_setupAbstraction(
        SWT_Abstraction* abstraction,
        const UpdateFuncs &updateFuncs,
        const int visiblePartWidgetId) {
    for(const auto &property : ca_mChildren) {
        auto newAbs = property->SWT_createAbstraction(updateFuncs,
                                                      visiblePartWidgetId);
        abstraction->addChildAbstraction(newAbs->ref<SWT_Abstraction>());
    }
}

FrameRange ComplexAnimator::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    for(const auto& child : ca_mChildren) {
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

void ComplexAnimator::SWT_setChildrenAncestorDisabled(const bool bT) {
    for(const auto& prop : ca_mChildren) {
        prop->SWT_setAncestorDisabled(bT);
    }
}

void ComplexAnimator::ca_insertChild(const qsptr<Property>& child,
                                     const int id) {
    const int cId = ca_mChildren.indexOf(child);
    if(cId != -1) {
        return ca_moveChildInList(child.get(), (cId < id ? id - 1 : id));
    }

    if(!ca_hasChildren()) {
        if(ca_mDisabledEmpty) SWT_setEnabled(true);
        if(ca_mHiddenEmpty) SWT_setVisible(true);
    }

    ca_mChildren.insert(id, child);
    child->setParent(this);
    child->prp_setInheritedFrameShift(prp_getTotalFrameShift(), this);
    if(child->drawsOnCanvas() ||
       child->SWT_isComplexAnimator()) {
        prp_updateCanvasProps();
    }

    const bool changeInfluence = !(SWT_isBoundingBox() &&
                                   child->SWT_isSound());

    if(child->SWT_isAnimator()) {
        const auto childAnimator = static_cast<Animator*>(child.get());
        connect(childAnimator, &Animator::anim_isRecordingChanged,
                this, &ComplexAnimator::ca_childIsRecordingChanged);
        connect(childAnimator, &Animator::anim_addedKey,
                this, &ComplexAnimator::ca_addDescendantsKey);
        connect(childAnimator, &Animator::anim_removedKey,
                this, &ComplexAnimator::ca_removeDescendantsKey);
        childAnimator->anim_addAllKeysToComplexAnimator(this);
        ca_childIsRecordingChanged();
        childAnimator->anim_setAbsFrame(anim_getCurrentAbsFrame());
    }
    if(changeInfluence){
        connect(child.data(), &Property::prp_absFrameRangeChanged,
                this, &ComplexAnimator::prp_afterChangedAbsRange);
    }

    child->SWT_setAncestorDisabled(SWT_isDisabled());
    SWT_addChildAt(child.get(), id);
    if(changeInfluence) {
        const auto childRange = child->prp_absInfluenceRange();
        const auto changedRange = childRange*prp_absInfluenceRange();
        prp_afterChangedAbsRange(changedRange);
    }

    emit ca_childAdded(child.get());
}

int ComplexAnimator::ca_getChildPropertyIndex(Property * const child) {
    for(int i = 0; i < ca_mChildren.count(); i++) {
        if(ca_mChildren.at(i) == child) return i;
    }
    return -1;
}

void ComplexAnimator::ca_updateDescendatKeyFrame(Key* key) {
    const auto& keys = anim_getKeys();
    for(const auto& ckey : keys) {
        const auto complexKey = static_cast<ComplexKey*>(ckey);
        if(complexKey->hasKey(key)) {
            complexKey->removeChildKey(key);
            if(complexKey->isEmpty()) anim_removeKey(complexKey->ref<ComplexKey>());
            ca_addDescendantsKey(key);
            break;
        }
    }
}

void ComplexAnimator::ca_execOnDescendants(const std::function<void (Property *)> &op) const {
    for(const auto& child : ca_mChildren) {
        op(child.get());
        if(child->SWT_isComplexAnimator()) {
            static_cast<ComplexAnimator*>(child.get())->ca_execOnDescendants(op);
        }
    }
}

void ComplexAnimator::ca_moveChildAbove(Property *move, Property *above) {
    const int indexFrom = ca_getChildPropertyIndex(move);
    int indexTo = ca_getChildPropertyIndex(above);
    if(indexFrom > indexTo) indexTo++;
    ca_moveChildInList(move, indexFrom, indexTo);
}

void ComplexAnimator::ca_moveChildBelow(Property *move, Property *below) {
    const int indexFrom = ca_getChildPropertyIndex(move);
    int indexTo = ca_getChildPropertyIndex(below);
    if(indexFrom < indexTo) indexTo--;
    ca_moveChildInList(move, indexFrom, indexTo);
}

void ComplexAnimator::ca_moveChildInList(Property* child, const int to) {
    const int from = ca_getChildPropertyIndex(child);
    if(from == -1) return;
    ca_moveChildInList(child, from, to);
}

void ComplexAnimator::ca_moveChildInList(Property* child,
                                         const int from, const int to) {
    if(from == to) return;
    const auto boundTo = qBound(0, to, ca_mChildren.count() - 1);
    ca_mChildren.move(from, boundTo);
    SWT_moveChildTo(child, boundTo);
    prp_afterWholeInfluenceRangeChanged();
    emit ca_childMoved(child);
}

void ComplexAnimator::ca_removeChild(const qsptr<Property> child) {
    const bool changeInfluence = !(SWT_isBoundingBox() &&
                                   child->SWT_isSound());
    const auto childRange = child->prp_absInfluenceRange();
    if(child->SWT_isAnimator()) {
        const auto aRemove = static_cast<Animator*>(child.get());
        aRemove->anim_removeAllKeysFromComplexAnimator(this);
    }
    disconnect(child.get(), nullptr, this, nullptr);

    SWT_removeChild(child.get());

    child->setParent(nullptr);
    ca_mChildren.removeAt(ca_getChildPropertyIndex(child.get()));
    if(child->drawsOnCanvas() ||
       child->SWT_isComplexAnimator()) {
        prp_updateCanvasProps();
    }
    ca_childIsRecordingChanged();

    if(changeInfluence) {
        const auto changedRange = childRange*prp_absInfluenceRange();
        prp_afterChangedAbsRange(changedRange);
    }

    if(!ca_hasChildren()) {
        if(ca_mDisabledEmpty) SWT_setEnabled(false);
        if(ca_mHiddenEmpty) SWT_setVisible(false);
    }

    emit ca_childRemoved(child.get());
}

void ComplexAnimator::ca_removeAllChildren() {
    for(int i = ca_mChildren.count() - 1; i >= 0; i--)
        ca_removeChild(qsptr<Property>(ca_mChildren.at(i)));
}

void ComplexAnimator::anim_addKeyAtRelFrame(const int relFrame) {
    for(const auto &property : ca_mChildren) {
        if(property->SWT_isAnimator()) {
            static_cast<Animator*>(property.get())->anim_addKeyAtRelFrame(relFrame);
        }
    }
}

void ComplexAnimator::ca_swapChildren(Property * const child1,
                                      Property * const child2) {
    const int id1 = ca_getChildPropertyIndex(child1);
    const int id2 = ca_getChildPropertyIndex(child2);
    ca_mChildren.swap(id1, id2);
    prp_afterWholeInfluenceRangeChanged();
    emit ca_childMoved(child1);
    emit ca_childMoved(child2);
}

bool ComplexAnimator::ca_hasChildren() const {
    return !ca_mChildren.isEmpty();
}

void ComplexAnimator::prp_startTransform() {
    for(const auto &property : ca_mChildren)
        property->prp_startTransform();
}

void ComplexAnimator::prp_afterFrameShiftChanged(const FrameRange &oldAbsRange,
                                                 const FrameRange &newAbsRange) {
    Animator::prp_afterFrameShiftChanged(oldAbsRange, newAbsRange);
    const int thisShift = prp_getTotalFrameShift();
    for(const auto &property : ca_mChildren)
        property->prp_setInheritedFrameShift(thisShift, this);
}

void ComplexAnimator::anim_setAbsFrame(const int frame) {
    //if(!anim_isDescendantRecording()) return;
    Animator::anim_setAbsFrame(frame);

    for(const auto &property : ca_mChildren) {
        if(!property->SWT_isAnimator()) continue;
        const auto anim = static_cast<Animator*>(property.get());
        anim->anim_setAbsFrame(frame);
    }
}

void ComplexAnimator::prp_finishTransform() {
    for(const auto &property : ca_mChildren)
        property->prp_finishTransform();
}

void ComplexAnimator::prp_cancelTransform() {
    for(const auto &property : ca_mChildren)
        property->prp_cancelTransform();
}

bool ComplexAnimator::anim_isDescendantRecording() const {
    return ca_mChildRecording;
}

QString ComplexAnimator::prp_getValueText() {
    return "";
}

void ComplexAnimator::anim_setRecording(const bool rec) {
    for(const auto &property : ca_mChildren) {
        if(!property->SWT_isAnimator()) continue;
        const auto anim = static_cast<Animator*>(property.get());
        anim->anim_setRecording(rec);
    }
    anim_setRecordingValue(rec);
}

void ComplexAnimator::ca_childIsRecordingChanged() {
    bool rec = true;
    bool childRec = false;
    for(const auto &property : ca_mChildren) {
        if(!property->SWT_isAnimator()) continue;
        const auto anim = static_cast<Animator*>(property.get());
        const bool isChildRec = anim->anim_isRecording();
        const bool isChildDescRec = anim->anim_isDescendantRecording();
        if(isChildDescRec) childRec = true;
        if(!isChildRec) rec = false;
    }
    rec = rec && childRec;
    if(childRec != ca_mChildRecording) {
        ca_mChildRecording = childRec;
        if(rec == anim_isRecording()) emit anim_isRecordingChanged();
    }
    if(rec != anim_isRecording()) {
        anim_setRecordingValue(rec);
    }
}

void ComplexAnimator::ca_addDescendantsKey(Key * const key) {
    auto collection = anim_getKeyAtAbsFrame<ComplexKey>(key->getAbsFrame());
    if(!collection) {
        auto newCollection = enve::make_shared<ComplexKey>(key->getAbsFrame(), this);
        collection = newCollection.get();
        anim_appendKey(newCollection);
    }
    collection->addChildKey(key);
}

void ComplexAnimator::ca_removeDescendantsKey(Key * const key) {
    const auto collection = anim_getKeyAtRelFrame<ComplexKey>(key->getRelFrame());
    if(!collection) return;
    collection->removeChildKey(key);
    if(collection->isEmpty()) anim_removeKey(collection->ref<ComplexKey>());
}

void ComplexAnimator::ca_addChild(const qsptr<Property> &child) {
    ca_insertChild(child, ca_getNumberOfChildren());
}

void ComplexAnimator::anim_shiftAllKeys(const int shift) {
    for(const auto &property : ca_mChildren) {
        if(!property->SWT_isAnimator()) continue;
        const auto anim = static_cast<Animator*>(property.get());
        anim->anim_shiftAllKeys(shift);
    }
}
