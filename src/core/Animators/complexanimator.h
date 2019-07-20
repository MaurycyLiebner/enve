#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "animator.h"
#include "key.h"

class ComplexKey;
class KeysClipboardContainer;
class QrealAnimator;

class ComplexAnimator : public Animator {
    friend class SelfRef;
protected:
    ComplexAnimator(const QString& name);
    void prp_setUpdater(const stdsptr<PropertyUpdater> &updater);
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

    void prp_afterFrameShiftChanged();

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    void anim_saveCurrentValueAsKey();
    bool anim_isDescendantRecording() const;
    virtual void ca_removeAllChildAnimators();

    void anim_addKeyAtRelFrame(const int relFrame) {
        for(const auto &property : ca_mChildAnimators) {
            if(property->SWT_isAnimator()) {
                GetAsPtr(property, Animator)->anim_addKeyAtRelFrame(relFrame);
            }
        }
    }

    void anim_setRecording(const bool rec);
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

    Property *ca_getFirstDescendantWithName(const QString &name);

    void ca_execOnDescendants(const std::function<void(Property*)>& op) const {
        for(const auto& child : ca_mChildAnimators) {
            op(child.get());
            if(child->SWT_isComplexAnimator()) {
                GetAsPtr(child, ComplexAnimator)->ca_execOnDescendants(op);
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
protected:
    void ca_addChildAnimator(const qsptr<Property> &childAnimator) {
        ca_insertChildAnimator(childAnimator, ca_getNumberOfChildren());
    }
    void ca_insertChildAnimator(const qsptr<Property> &childAnimator,
                                const int id);
    void ca_removeChildAnimator(const qsptr<Property> &removeAnimator);

    template <typename T = Property>
    qsptr<T> ca_takeChildAt(const int i) {
        if(i < 0 || i >= ca_getNumberOfChildren())
            RuntimeThrow("Index outside of range");
        if(mHiddenEmpty && ca_getNumberOfChildren() == 1) {
            SWT_setEnabled(false);
            SWT_setVisible(false);
        }
        return GetAsSPtrTemplated(ca_mChildAnimators.takeAt(i), T);
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
    friend class StdSelfRef;
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
