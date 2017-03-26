#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "Animators/qrealanimator.h"

class ComplexKey;
class ComplexAnimatorItemWidgetContainer;

class ComplexAnimator : public QrealAnimator
{
public:
    ComplexAnimator();
    ~ComplexAnimator();

    QMatrix qra_getCurrentValue();

    void ca_addDescendantsKey(Key *key);
    ComplexKey *getKeyCollectionAtAbsFrame(int frame);
    ComplexKey *getKeyCollectionAtRelFrame(int frame);

    void ca_removeDescendantsKey(Key *key);
    void drawChildAnimatorKeys(QPainter *p,
                               qreal pixelsPerFrame, qreal startY,
                               int startFrame, int endFrame);
    qreal qra_clampValue(qreal value);
    void addChildAnimator(Property *childAnimator);
    void removeChildAnimator(Property *removeAnimator);
    void prp_startTransform();
    void prp_setUpdater(AnimatorUpdater *updater);
    void prp_setAbsFrame(int frame);

    void prp_retrieveSavedValue();
    void prp_finishTransform();
    void prp_cancelTransform();

    void prp_setRecording(bool rec);

    virtual void childAnimatorIsRecordingChanged();
    void setRecordingValue(bool rec);

    bool prp_isDescendantRecording();
    QString prp_getValueText();
    void swapChildAnimators(Property *animator1, Property *animator2);
    void prp_clearFromGraphView();

    bool hasChildAnimators();

    void prp_setTransformed(bool bT);

    void changeChildAnimatorZ(const int &oldIndex,
                              const int &newIndex);
    int getNumberOfChildren();
    Property *getChildAt(const int &i);

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) {
        if(hasChildAnimators()) {
            return QrealAnimator::SWT_shouldBeVisible(rules,
                                                      parentSatisfies,
                                                      parentMainTarget);
        } else {
            return false;
        }
    }

    SWT_Type SWT_getType() { return SWT_ComplexAnimator; }
    void anim_drawKey(QPainter *p, QrealKey *key,
                 const qreal &pixelsPerFrame,
                 const qreal &drawY, const int &startFrame);
    void moveChildInList(Property *child,
                         int from, int to,
                         bool saveUndoRedo = true);
    void moveChildBelow(Property *move, Property *below);
    void moveChildAbove(Property *move, Property *above);
protected:
    bool mChildAnimatorRecording = false;
    QList<Property*> mChildAnimators;
    qreal mMinMoveValue;
    qreal mMaxMoveValue;
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

    void setRelFrame(int frame);

    void mergeWith(Key *key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

    void startFrameTransform();
    void finishFrameTransform();
    void copyToContainer(KeysClipboardContainer *container);
    void cancelFrameTransform();
    //void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame, const qreal &scaleFactor);
    //QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    bool areAllChildrenSelected() {
        foreach(Key *key, mKeys) {
            if(key->isSelected() ||
               key->areAllChildrenSelected()) continue;
            return false;
        }

        return true;
    }
    void removeFromSelection(QList<Key *> *selectedKeys);
    void addToSelection(QList<Key *> *selectedKeys);
private:
    QList<Key*> mKeys;
};

#endif // COMPLEXANIMATOR_H
