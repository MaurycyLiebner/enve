#include "complexanimator.h"

ComplexAnimator::ComplexAnimator() :
    QrealAnimator()
{

}

#include <QDebug>
void ComplexAnimator::drawKeys(QPainter *p, qreal pixelsPerFrame,
                               qreal startX, qreal startY, qreal height,
                               int startFrame, int endFrame,
                               bool detailedView)
{
    QrealAnimator::drawKeys(p, pixelsPerFrame, startX, startY, height,
                            startFrame, endFrame, detailedView);

    if(detailedView) {
        drawChildAnimatorKeys();
    }
}

void ComplexAnimator::drawDiagram(QPainter *p,
                                  qreal pixelsPerFrame, qreal pixelsPerValue,
                                  int startFrame, int endFrame,
                                  bool detailedView) {
//    qreal radius = pixelsPerFrame*0.5;
//    foreach(KeyCollection *key, mKeys) {
//        if(key->frame >= startFrame && key->frame <= endFrame) {
//            p->drawEllipse(
//                    QPointF((key->frame - startFrame)*pixelsPerFrame, centerY),
//                    radius, radius);
//        }
    //    }
}

qreal ComplexAnimator::clampValue(qreal value)
{
    return clamp(value, mMinMoveValue, mMaxMoveValue);
}

ComplexKey *ComplexAnimator::getKeyCollectionAtFrame(int frame) {
    return (ComplexKey *) getKeyAtFrame(frame);
}

void ComplexAnimator::updateMinAndMaxMove(QrealKey *key) {
    QrealAnimator::updateMinAndMaxMove(key);
    mMinMoveValue = mMinMoveFrame;
    mMaxMoveValue = mMaxMoveFrame;
}

void ComplexAnimator::addChildQrealKey(QrealKey *key)
{
    ComplexKey *collection = getKeyCollectionAtFrame(key->getFrame() );
    if(collection == NULL) {
        collection = new ComplexKey(key->getFrame(), this );
        appendKey(collection);
        sortKeys();
    }
    collection->addAnimatorKey(key);
}

void ComplexAnimator::removeChildQrealKey(QrealKey *key)
{
    ComplexKey *collection = getKeyCollectionAtFrame(key->getFrame() );
    collection->removeAnimatorKey(key);
    if(collection->isEmpty() ) {
        removeKey(collection);
    }
}

ComplexKey::ComplexKey(int frameT, ComplexAnimator *parentAnimator) :
    QrealKey(frameT, parentAnimator, frameT) {
}

void ComplexKey::setStartValue(qreal value) {
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

void ComplexKey::setEndValue(qreal value) {
    QrealKey::setEndValue(value);

    foreach(QrealKey *key, mKeys) {
        if(key->hasNextKey() ) {
            key->setEndValue(value *
                             (key->getNextKeyValue() - key->getValue() ) /
                             (getNextKeyValue() - getValue()) + key->getValue() );
        }
    }
}

void ComplexKey::setStartFrame(qreal startFrame) {
    QrealKey::setStartFrame(startFrame);

    foreach(QrealKey *key, mKeys) {
        key->setStartFrame(startFrame);
    }
}

void ComplexKey::setEndFrame(qreal endFrame) {
    QrealKey::setEndFrame(endFrame);

    foreach(QrealKey *key, mKeys) {
        key->setEndFrame(endFrame);
    }
}

void ComplexKey::addAnimatorKey(QrealKey *key) {
    mKeys << key;
    key->incNumberPointers();
}

void ComplexKey::addOrMergeKey(QrealKey *keyAdd) {
    foreach(QrealKey *key, mKeys) {
        if(key->getParentAnimator() == keyAdd->getParentAnimator() ) {
            key->mergeWith(keyAdd);
            return;
        }
    }
    addAnimatorKey(keyAdd);
}

void ComplexKey::removeAnimatorKey(QrealKey *key) {
    if(mKeys.removeOne(key) ) {
        key->decNumberPointers();
    }
}

bool ComplexKey::isEmpty() {
    return mKeys.isEmpty();
}

void ComplexKey::setValue(qreal) { QrealKey::setValue(mFrame); }

qreal ComplexKey::getValue() { return mFrame; }

void ComplexKey::setFrame(int frame) {
    QrealKey::setFrame(frame);
    QrealKey::setValue(frame);

    foreach(QrealKey *key, mKeys) {
        key->setFrame(frame);
    }
}

void ComplexKey::mergeWith(QrealKey *key) {
    ((ComplexKey*) key)->margeAllKeysToKey(this);
    key->removeFromAnimator();
}

void ComplexKey::margeAllKeysToKey(ComplexKey *target) {
    QList<QrealKey*> keys = mKeys;
    foreach(QrealKey *key, keys) {
        removeAnimatorKey(key);
        target->addOrMergeKey(key); // this might be deleted
    }
}

bool ComplexKey::isDescendantSelected() {
    if(isSelected()) return true;
    foreach(QrealKey *key, mKeys) {
        if(key->isDescendantSelected()) return true;
    }
    return false;
}
