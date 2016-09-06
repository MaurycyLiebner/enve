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
