#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "Animators/animator.h"
#include "key.h"

class ComplexKey;
class KeysClipboardContainer;

class ComplexAnimator : public Animator
{
    Q_OBJECT
public:
    ComplexAnimator();
    ~ComplexAnimator();

    void ca_addChildAnimator(Property *childAnimator);
    void ca_removeChildAnimator(Property *removeAnimator);
    void ca_swapChildAnimators(Property *animator1, Property *animator2);
    void ca_moveChildInList(Property *child,
                            const int &from,
                            const int &to,
                            const bool &saveUndoRedo = true);
    void ca_moveChildBelow(Property *move, Property *below);
    void ca_moveChildAbove(Property *move, Property *above);

    void prp_startTransform();
    void prp_setUpdater(AnimatorUpdater *updater);
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
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);

    bool SWT_isComplexAnimator();

    void anim_drawKey(QPainter *p,
                      Key *key,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame);

    void anim_loadKeysFromSql(const int &qrealAnimatorId);

    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator = NULL);
    int getChildPropertyIndex(Property *child);

    void ca_updateDescendatKeyFrame(Key *key);
public slots:
    void prp_setRecording(const bool &rec);

    virtual void ca_addDescendantsKey(Key *key);
    virtual void ca_removeDescendantsKey(Key *key);
    virtual void ca_childAnimatorIsRecordingChanged();
protected:
    ComplexKey *ca_getKeyCollectionAtAbsFrame(const int &frame);
    ComplexKey *ca_getKeyCollectionAtRelFrame(const int &frame);
    bool ca_mChildAnimatorRecording = false;
    QList<QSharedPointer<Property> > ca_mChildAnimators;
};

class ComplexKey : public Key
{
public:
    ComplexKey(ComplexAnimator *parentAnimator);

    void addAnimatorKey(Key* key);

    void addOrMergeKey(Key *keyAdd);

    void deleteKey();

    void removeAnimatorKey(Key *key);

    bool isEmpty();

    void setRelFrame(const int &frame);

    void mergeWith(Key *key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

    void startFrameTransform();
    void finishFrameTransform();
    void copyToContainer(KeysClipboardContainer *container);
    void cancelFrameTransform();
    //void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame, const qreal &scaleFactor);
    //QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    bool areAllChildrenSelected();
    void removeFromSelection(QList<Key *> *selectedKeys);
    void addToSelection(QList<Key *> *selectedKeys);

    bool hasKey(Key *key);

    bool differsFromKey(Key *key);

    int getChildKeysCount();

    bool hasSameKey(Key *otherKey);
private:
    QList<Key*> mKeys;
};

#endif // COMPLEXANIMATOR_H
