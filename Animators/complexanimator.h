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
    ComplexKey *getKeyCollectionAtFrame(int frame);
    void removeChildQrealKey(QrealKey *key);
    void drawChildAnimatorKeys(QPainter *p,
                               qreal pixelsPerFrame, qreal startY,
                               int startFrame, int endFrame);
    qreal clampValue(qreal value);
    void addChildAnimator(QrealAnimator *childAnimator);
    void removeChildAnimator(QrealAnimator *removeAnimator);
    void startTransform();
    void setUpdater(AnimatorUpdater *updater);
    void setFrame(int frame);
    void sortKeys();
    void updateKeysPath();

    void retrieveSavedValue();
    void finishTransform();
    void cancelTransform();

    void setRecording(bool rec);

    virtual void childAnimatorIsRecordingChanged();
    void setRecordingValue(bool rec);

    bool isDescendantRecording();
    QString getValueText();
    void swapChildAnimators(QrealAnimator *animator1, QrealAnimator *animator2);
    void clearFromGraphView();

    bool hasChildAnimators();

    void setTransformed(bool bT);

    void changeChildAnimatorZ(const int &oldIndex,
                              const int &newIndex);
    int getNumberOfChildren();
    QrealAnimator *getChildAt(const int &i);

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);
    SWT_Type SWT_getType() { return SWT_ComplexAnimator; }
    void drawKey(QPainter *p, QrealKey *key,
                 const qreal &pixelsPerFrame,
                 const qreal &drawY, const int &startFrame);
protected:
    bool mChildAnimatorRecording = false;
    QList<QrealAnimator*> mChildAnimators;
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

    void setValue(qreal, bool finish = false);

    qreal getValue();

    void setFrame(int frame);

    void mergeWith(QrealKey *key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

    void startFrameTransform();
    void finishFrameTransform();
    void copyToContainer(KeysClipboardContainer *container);
    void cancelFrameTransform();
    //void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame, const qreal &scaleFactor);
    QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);
private:
    QList<QrealKey*> mKeys;
};

#endif // COMPLEXANIMATOR_H
