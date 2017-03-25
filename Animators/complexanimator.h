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

    QMatrix getCurrentValue();

    void addChildQrealKey(QrealKey *key);
    ComplexKey *getKeyCollectionAtAbsFrame(int frame);
    ComplexKey *getKeyCollectionAtRelFrame(int frame);

    void removeChildQrealKey(QrealKey *key);
    void drawChildAnimatorKeys(QPainter *p,
                               qreal pixelsPerFrame, qreal startY,
                               int startFrame, int endFrame);
    qreal clampValue(qreal value);
    void addChildAnimator(Property *childAnimator);
    void removeChildAnimator(Property *removeAnimator);
    void startTransform();
    void setUpdater(AnimatorUpdater *updater);
    void setAbsFrame(int frame);

    void retrieveSavedValue();
    void finishTransform();
    void cancelTransform();

    void setRecording(bool rec);

    virtual void childAnimatorIsRecordingChanged();
    void setRecordingValue(bool rec);

    bool isDescendantRecording();
    QString getValueText();
    void swapChildAnimators(Property *animator1, Property *animator2);
    void clearFromGraphView();

    bool hasChildAnimators();

    void setTransformed(bool bT);

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
    void drawKey(QPainter *p, QrealKey *key,
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

class ComplexKey : public QrealKey
{
public:
    ComplexKey(ComplexAnimator *parentAnimator);

    void setStartValue(qreal);

    void setEndValue(qreal);

    void setStartFrame(qreal startFrame);

    void setEndFrame(qreal endFrame);

    void addAnimatorKey(QrealKey* key);

    void addOrMergeKey(QrealKey *keyAdd);

    void deleteKey();

    void setCtrlsMode(CtrlsMode mode);

    void setEndEnabled(bool bT);

    void setStartEnabled(bool bT);

    void removeAnimatorKey(QrealKey *key);

    bool isEmpty();

    void setRelFrame(int frame);

    void mergeWith(QrealKey *key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

    void startFrameTransform();
    void finishFrameTransform();
    void copyToContainer(KeysClipboardContainer *container);
    void cancelFrameTransform();
    //void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame, const qreal &scaleFactor);
    QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    bool areAllChildrenSelected() {
        foreach(QrealKey *key, mKeys) {
            if(key->isSelected() ||
               key->areAllChildrenSelected()) continue;
            return false;
        }

        return true;
    }
    void removeFromSelection(QList<QrealKey *> *selectedKeys);
    void addToSelection(QList<QrealKey *> *selectedKeys);
private:
    QList<QrealKey*> mKeys;
};

#endif // COMPLEXANIMATOR_H
