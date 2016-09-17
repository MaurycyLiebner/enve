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
    void drawDiagram(QPainter *p,
                     qreal pixelsPerFrame, qreal pixelsPerValue,
                     int startFrame, int endFrame, bool detailedView);
    qreal clampValue(qreal value);
    void updateMinAndMaxMove(QrealKey *key);
protected:
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
