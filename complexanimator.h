#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "qrealanimator.h"

class ComplexKey;

class ComplexAnimator : public QrealAnimator
{
public:
    ComplexAnimator();
    QMatrix getCurrentValue();

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  qreal startX, qreal startY, qreal height,
                  int startFrame, int endFrame,
                  bool detailedView);

    void addChildQrealKey(QrealKey *key);
    ComplexKey *getKeyCollectionAtFrame(int frame);
    void removeChildQrealKey(QrealKey *key);
    virtual void drawChildAnimatorKeys() {}
    qreal clampValue(qreal value);
    void addChildAnimator(QrealAnimator *childAnimator);
    void removeChildAnimator(QrealAnimator *removeAnimator);
    void startTransform();
    void setUpdater(AnimatorUpdater *updater);
    void setFrame(int frame);
    void sortKeys();
    void updateKeysPath();
    qreal getBoxesListHeight();
    void drawBoxesList(QPainter *p,
                       qreal drawX, qreal drawY,
                       qreal pixelsPerFrame,
                       int startFrame, int endFrame,
                       bool animationBar);
    QrealKey *getKeyAtPos(qreal relX, qreal relY, int minViewedFrame, qreal pixelsPerFrame);
    void getKeysInRect(QRectF selectionRect, int minViewedFrame,
                       qreal pixelsPerFrame,
                       QList<QrealKey *> *keysList);
    void handleListItemMousePress(qreal boxesListX, qreal relX, qreal relY, QMouseEvent *event);
    void retrieveSavedValue();
    void finishTransform();
    void cancelTransform();

    void setRecording(bool rec);

    void childAnimatorIsRecordingChanged();
    void setRecordingValue(bool rec);

    bool isDescendantRecording();
    QString getValueText();
    void swapChildAnimators(QrealAnimator *animator1, QrealAnimator *animator2);
protected:
    bool mChildAnimatorRecording = false;
    QList<QrealAnimator*> mChildAnimators;
    qreal mMinMoveValue;
    qreal mMaxMoveValue;
};

class ComplexKey : public QrealKey
{
public:
    ComplexKey(int frameT, ComplexAnimator *parentAnimator);

    void setStartValue(qreal value);

    void setEndValue(qreal value);

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

    void setValue(qreal);

    qreal getValue();

    void setFrame(int frame);

    void mergeWith(QrealKey *key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

private:
    QList<QrealKey*> mKeys;
};

#endif // COMPLEXANIMATOR_H
