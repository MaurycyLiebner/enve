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

#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "animator.h"
#include "key.h"

class ComplexKey;
class KeysClipboard;
class QrealAnimator;

class ComplexAnimator : public Animator {
    e_OBJECT
    Q_OBJECT
protected:
    ComplexAnimator(const QString& name);
public:
    stdsptr<Key> createKey() final {
        return nullptr;
    }

    void SWT_setupAbstraction(SWT_Abstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;

    bool SWT_isComplexAnimator() const;

    void SWT_setChildrenAncestorDisabled(const bool bT) {
        for(const auto& prop : ca_mChildAnimators) {
            prop->SWT_setAncestorDisabled(bT);
        }
    }

    void prp_startTransform();
    void anim_setAbsFrame(const int frame);

    void prp_finishTransform();
    void prp_cancelTransform();
    void prp_setTransformed(const bool bT);
    QString prp_getValueText();

    void prp_afterFrameShiftChanged(const FrameRange& oldAbsRange,
                                    const FrameRange& newAbsRange);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    bool anim_isDescendantRecording() const;
    virtual void ca_removeAllChildAnimators();

    void anim_addKeyAtRelFrame(const int relFrame) {
        for(const auto &property : ca_mChildAnimators) {
            if(property->SWT_isAnimator()) {
                static_cast<Animator*>(property.get())->anim_addKeyAtRelFrame(relFrame);
            }
        }
    }

    void anim_setRecording(const bool rec);
    void anim_shiftAllKeys(const int shift);

    virtual void ca_childAnimatorIsRecordingChanged();

    void ca_swapChildAnimators(Property * const animator1,
                               Property * const animator2);
    void ca_moveChildInList(Property *child, const int to);
    void ca_moveChildInList(Property *child, const int from, const int to);
    void ca_moveChildBelow(Property *move, Property *below);
    void ca_moveChildAbove(Property *move, Property *above);

    bool hasChildAnimators() const;

    void ca_changeChildAnimatorZ(const int oldIndex, const int newIndex);
    int ca_getNumberOfChildren() const;

    template <typename T = Property>
    T *ca_getChildAt(const int i) const {
        if(i < 0 || i >= ca_getNumberOfChildren())
            RuntimeThrow("Index outside of range");
        return static_cast<T*>(ca_mChildAnimators.at(i).data());
    }

    int getChildPropertyIndex(Property * const child);

    void ca_updateDescendatKeyFrame(Key* key);

    template <class T = Property>
    T *ca_getFirstDescendant(const std::function<bool(T*)>& tester) const {
        for(const auto &prop : ca_mChildAnimators) {
            const auto target = dynamic_cast<T*>(prop.data());
            if(target && tester(target)) return target;
            else if(prop->SWT_isComplexAnimator()) {
                const auto ca = static_cast<ComplexAnimator*>(prop.get());
                const auto desc = ca->ca_getFirstDescendant<T>(tester);
                if(desc) return desc;
            }
        }
        return nullptr;
    }

    template <class T = Property>
    T *ca_getFirstDescendant() const {
        return ca_getFirstDescendant<T>([](T* const prop) {
            Q_UNUSED(prop)
            return true;
        });
    }

    template <class T = Property>
    T *ca_getFirstDescendantWithName(const QString &name) {
        return ca_getFirstDescendant<T>([name](T* const prop) {
            return prop->prp_getName() == name;
        });
    }

    void ca_execOnDescendants(const std::function<void(Property*)>& op) const {
        for(const auto& child : ca_mChildAnimators) {
            op(child.get());
            if(child->SWT_isComplexAnimator()) {
                static_cast<ComplexAnimator*>(child.get())->ca_execOnDescendants(op);
            }
        }
    }

    Property* getPropertyForGUI() const {
        return mPropertyGUI;
    }

    void setPropertyForGUI(Property * const prop) {
        mPropertyGUI = prop;
    }

    void ca_addDescendantsKey(Key * const key);
    void ca_removeDescendantsKey(Key * const key);
signals:
    void childAdded(Property*);
    void childRemoved(Property*);
protected:
    void ca_addChild(const qsptr<Property> &childAnimator) {
        ca_insertChild(childAnimator, ca_getNumberOfChildren());
    }
    void ca_insertChild(const qsptr<Property> &child, const int id);
    void ca_removeChild(const qsptr<Property> child);

    template <typename T = Property>
    qsptr<T> ca_takeChildAt(const int i) {
        const auto result = ca_mChildAnimators.at(i);
        ca_removeChild(result);
        return result->ref<T>();
    }

    void ca_prependChildAnimator(Property *childAnimator,
                                 const qsptr<Property>& prependWith);
    void ca_replaceChildAnimator(const qsptr<Property> &childAnimator,
                                 const qsptr<Property>& replaceWith);

    QList<qsptr<Property>> ca_mChildAnimators;

    void makeHiddenWhenEmpty() {
        if(mHiddenEmpty) return;
        mHiddenEmpty = true;
        if(!hasChildAnimators()) {
            SWT_setEnabled(false);
            SWT_setVisible(false);
        }
    }
private:
    bool mHiddenEmpty = false;
    qptr<Property> mPropertyGUI;
    bool ca_mChildAnimatorRecording = false;
};

class ComplexKey : public Key {
    e_OBJECT
public:
    void deleteKey();
    bool isDescendantSelected() const;

    void startFrameTransform();
    void finishFrameTransform();
    void cancelFrameTransform();

    bool isSelected() const;
    void addToSelection(QList<Animator *> &selectedAnimators);
    void removeFromSelection(QList<Animator*> &selectedAnimators);
    bool differsFromKey(Key *otherKey) const;

    void addAnimatorKey(Key * const key);

    void removeAnimatorKey(Key * const key);

    bool isEmpty() const;

//    void setRelFrame(const int frame);

    void moveAllKeysTo(ComplexKey * const target);
    bool hasKey(Key *key) const;

    int getChildKeysCount() const;
    bool hasSameKey(Key *otherKey) const;
protected:
    ComplexKey(const int absFrame,
               ComplexAnimator * const parentAnimator);
private:
    QList<Key*> mKeys;
};

#endif // COMPLEXANIMATOR_H
