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

    void ca_addChildAnimator(const PropertyQSPtr &childAnimator,
                             const int &id = INT_MAX);
    PropertyQSPtr ca_removeChildAnimator(const PropertyQSPtr &removeAnimator);
    void ca_swapChildAnimators(Property *animator1, Property *animator2);
    void ca_moveChildInList(Property *child,
                            const int &from,
                            const int &to,
                            const bool &saveUndoRedo = true);
    void ca_moveChildBelow(Property *move,
                           Property *below);
    void ca_moveChildAbove(Property *move,
                           Property *above,
                           const bool &saveUndoRedo = true);

    void prp_startTransform();
    void prp_setUpdater(const AnimatorUpdaterSPtr& updater);
    void prp_setAbsFrame(const int &frame);

    void prp_retrieveSavedValue();
    void prp_finishTransform();
    void prp_cancelTransform();

    bool prp_isDescendantRecording();
    QString prp_getValueText();
    void prp_clearFromGraphView();

    bool hasChildAnimators();

    void prp_setTransformed(const bool &bT);

    void ca_changeChildAnimatorZ(const int &oldIndex,
                                 const int &newIndex);
    int ca_getNumberOfChildren();
    Property *ca_getChildAt(const int &i);

    void SWT_addChildrenAbstractions(
            const SingleWidgetAbstractionSPtr& abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);

    bool SWT_isComplexAnimator();

    void anim_drawKey(QPainter *p,
                      Key* key,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame);


    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator = nullptr);
    int getChildPropertyIndex(Property *child);

    void ca_updateDescendatKeyFrame(const KeySPtr &key);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    void anim_saveCurrentValueAsKey();
    virtual void ca_removeAllChildAnimators();
    Property *ca_getFirstDescendantWithName(const QString &name);
    QrealAnimator *getQrealAnimatorIfIsTheOnlyOne();

    void SWT_setChildrenAncestorDisabled(const bool &bT) {
        Q_FOREACH(const QSharedPointer<Property> &prop, ca_mChildAnimators) {
            prop->SWT_setAncestorDisabled(bT);
        }
    }
public slots:
    void ca_prependChildAnimator(Property *childAnimator,
                                 const PropertyQSPtr& prependWith);
    void ca_replaceChildAnimator(const PropertyQSPtr &childAnimator,
                                 const PropertyQSPtr& replaceWith);
    void prp_setRecording(const bool &rec);

    virtual void ca_addDescendantsKey(const KeySPtr &key);
    virtual void ca_removeDescendantsKey(const KeySPtr &key);
    virtual void ca_childAnimatorIsRecordingChanged();
protected:
    ComplexKey *ca_getKeyCollectionAtAbsFrame(const int &frame);
    ComplexKey *ca_getKeyCollectionAtRelFrame(const int &frame);
    bool ca_mChildAnimatorRecording = false;
    QList<PropertyQSPtr> ca_mChildAnimators;
};

class ComplexKey : public Key {
public:
    ComplexKey(const ComplexAnimatorQSPtr &parentAnimator);

    void addAnimatorKey(const KeySPtr &key);

    void addOrMergeKey(const KeySPtr &keyAdd);

    void deleteKey();

    void removeAnimatorKey(const KeySPtr &key);

    bool isEmpty();

    void setRelFrame(const int &frame);

    void mergeWith(const KeySPtr &key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

    void startFrameTransform();
    void finishFrameTransform();
    void cancelFrameTransform();
    //void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame, const qreal &scaleFactor);
    //QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    bool areAllChildrenSelected();
    void removeFromSelection(QList<Key *> &selectedKeys);
    void addToSelection(QList<Key *> &selectedKeys);

    bool hasKey(Key *key);

    bool differsFromKey(Key *otherKey);

    int getChildKeysCount();

    bool hasSameKey(Key *otherKey);
private:
    QList<KeySPtr> mKeys;
};

#endif // COMPLEXANIMATOR_H
