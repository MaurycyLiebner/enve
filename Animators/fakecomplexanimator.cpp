#include "fakecomplexanimator.h"
#include <QPainter>

FakeComplexAnimator::FakeComplexAnimator(Property *target) {
    mTarget = target;
}

Property *FakeComplexAnimator::getTarget() {
    return mTarget;
}

void FakeComplexAnimator::prp_drawKeys(QPainter *p,
                                       const qreal &pixelsPerFrame,
                                       const qreal &drawY,
                                       const int &startFrame,
                                       const int &endFrame) {
    mTarget->prp_drawKeys(p, pixelsPerFrame, drawY,
                          startFrame, endFrame);
    ComplexAnimator::prp_drawKeys(p, pixelsPerFrame, drawY,
                                  startFrame, endFrame);
}

Key *FakeComplexAnimator::prp_getKeyAtPos(const qreal &relX,
                                          const int &minViewedFrame,
                                          const qreal &pixelsPerFrame) {
    Key *key = ComplexAnimator::prp_getKeyAtPos(relX, minViewedFrame,
                                                pixelsPerFrame);
    if(key != nullptr) return key;
    return mTarget->prp_getKeyAtPos(relX, minViewedFrame,
                                    pixelsPerFrame);
}

void FakeComplexAnimator::prp_getKeysInRect(const QRectF &selectionRect, const qreal &pixelsPerFrame, QList<Key *> *keysList) {
    mTarget->prp_getKeysInRect(selectionRect, pixelsPerFrame, keysList);
    ComplexAnimator::prp_getKeysInRect(selectionRect, pixelsPerFrame, keysList);
}

bool FakeComplexAnimator::SWT_isFakeComplexAnimator() { return true; }
