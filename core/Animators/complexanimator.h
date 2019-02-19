#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "Animators/animator.h"
#include "key.h"

class ComplexKey;
class KeysClipboardContainer;
class QrealAnimator;

class ComplexAnimator : public Animator {
    Q_OBJECT
    friend class SelfRef;
public:
    ComplexAnimator(const QString& name);
    ~ComplexAnimator();

    void ca_addChildAnimator(const qsptr<Property> &childAnimator,
                             const int &id = FrameRange::EMAX);
    void ca_removeChildAnimator(const qsptr<Property> &removeAnimator);
    void ca_swapChildAnimators(Property *animator1, Property *animator2);
    void ca_moveChildInList(Property *child, const int &from, const int &to);
    void ca_moveChildBelow(Property *move, Property *below);
    void ca_moveChildAbove(Property *move, Property *above);

    void prp_startTransform();
    void prp_setUpdater(const stdsptr<PropertyUpdater>& updater);
    void anim_setAbsFrame(const int &frame);

    void prp_retrieveSavedValue();
    void prp_finishTransform();
    void prp_cancelTransform();

    bool anim_isDescendantRecording() const;
    QString prp_getValueText();

    bool hasChildAnimators() const;

    void prp_setTransformed(const bool &bT);

    void ca_changeChildAnimatorZ(const int &oldIndex, const int &newIndex);
    int ca_getNumberOfChildren();
    Property *ca_getChildAt(const int &i);

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int& visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) const;

    bool SWT_isComplexAnimator() const;

    void anim_drawKey(QPainter *p,
                      Key* key,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &rowHeight,
                      const int &keyRectSize);


    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator = nullptr);
    int getChildPropertyIndex(Property *child);

    void ca_updateDescendatKeyFrame(Key* key);
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
    void anim_saveCurrentValueAsKey();
    virtual void ca_removeAllChildAnimators();
    Property *ca_getFirstDescendantWithName(const QString &name);

    template <class T = Property>
    T *getPropertyIfIsTheOnlyOne(bool (Property::*tester)() const) {
        if(ca_mChildAnimators.count() == 1) {
            Property* prop = ca_mChildAnimators.first().get();
            if((prop->*tester)()) {
                return static_cast<T*>(prop);
            }
        }
        return nullptr;
    }

    void SWT_setChildrenAncestorDisabled(const bool &bT) {
        for(const auto& prop : ca_mChildAnimators) {
            prop->SWT_setAncestorDisabled(bT);
        }
    }
public slots:
    void ca_prependChildAnimator(Property *childAnimator,
                                 const qsptr<Property>& prependWith);
    void ca_replaceChildAnimator(const qsptr<Property> &childAnimator,
                                 const qsptr<Property>& replaceWith);
    void anim_setRecording(const bool &rec);

    void ca_addDescendantsKey(Key * const key);
    void ca_removeDescendantsKey(Key * const key);
    virtual void ca_childAnimatorIsRecordingChanged();
protected:
    ComplexKey *ca_getKeyCollectionAtAbsFrame(const int &frame);
    ComplexKey *ca_getKeyCollectionAtRelFrame(const int &frame);
    bool ca_mChildAnimatorRecording = false;
    QList<qsptr<Property>> ca_mChildAnimators;
};

class ComplexKey : public Key {
    friend class StdSelfRef;
public:
    void addAnimatorKey(Key * const key);

    void addOrMergeKey(const stdsptr<Key> &keyAdd);

    void deleteKey();

    void removeAnimatorKey(Key * const key);

    bool isEmpty() const;

//    void setRelFrame(const int &frame);

    void mergeWith(const stdsptr<Key> &key);

    void margeAllKeysToKey(ComplexKey * const target);

    bool isDescendantSelected() const;

    void startFrameTransform();
    void finishFrameTransform();
    void cancelFrameTransform();

    bool isSelected() const;
    void addToSelection(QList<qptr<Animator>> &selectedAnimators);
    void removeFromSelection(QList<qptr<Animator>> &selectedAnimators);

    bool hasKey(Key *key) const;

    bool differsFromKey(Key *otherKey) const;

    int getChildKeysCount() const;

    bool hasSameKey(Key *otherKey) const;
protected:
    ComplexKey(const int& absFrame,
               ComplexAnimator * const parentAnimator);
private:
    QList<stdptr<Key>> mKeys;
};

#endif // COMPLEXANIMATOR_H
