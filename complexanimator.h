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
    ComplexKey(int frameT, ComplexAnimator *parentAnimator) :
        QrealKey(frameT, parentAnimator, frameT) {
    }

    void setStartValue(qreal value) {
        QrealKey::setStartValue(value);

        foreach(QrealKey *key, mKeys) {
            if(key->hasPrevKey() ) {
                qreal prevVal = key->getPrevKeyValue();
                key->setStartValue(value *
                                   (key->getValue() - prevVal ) /
                                   (getValue() - getPrevKeyValue()) + prevVal );
            }
        }
    }

    void setEndValue(qreal value) {
        QrealKey::setEndValue(value);

        foreach(QrealKey *key, mKeys) {
            if(key->hasNextKey() ) {
                key->setEndValue(value *
                                (key->getNextKeyValue() - key->getValue() ) /
                                (getNextKeyValue() - getValue()) + key->getValue() );
            }
        }
    }

    void setStartFrame(qreal startFrame) {
        QrealKey::setStartFrame(startFrame);

        foreach(QrealKey *key, mKeys) {
            key->setStartFrame(startFrame);
        }
    }

    void setEndFrame(qreal endFrame) {
        QrealKey::setEndFrame(endFrame);

        foreach(QrealKey *key, mKeys) {
            key->setEndFrame(endFrame);
        }
    }

    void addAnimatorKey(QrealKey* key) {
        mKeys << key;
    }

    void removeAnimatorKey(QrealKey *key) {
        mKeys.removeOne(key);
    }

    bool isEmpty() {
        return mKeys.isEmpty();
    }

    void setValue(qreal) { QrealKey::setValue(mFrame); }

    qreal getValue() { return mFrame; }

    void setFrame(int frame) {
        QrealKey::setFrame(frame);
        QrealKey::setValue(frame);

        foreach(QrealKey *key, mKeys) {
            key->setFrame(frame);
        }
    }
private:
    QList<QrealKey*> mKeys;
};

#endif // COMPLEXANIMATOR_H
