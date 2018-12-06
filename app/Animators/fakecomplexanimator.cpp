#include "fakecomplexanimator.h"
#include <QPainter>

FakeComplexAnimator::FakeComplexAnimator(const QString &name, Property *target) :
    ComplexAnimator(name) {
    mTarget = target;
}

Property *FakeComplexAnimator::getTarget() {
    return mTarget;
}

void FakeComplexAnimator::prp_drawKeys(QPainter *p,
                                       const qreal &pixelsPerFrame,
                                       const qreal &drawY,
                                       const int &startFrame,
                                       const int &endFrame,
                                       const int &rowHeight,
                                       const int &keyRectSize) {
    mTarget->prp_drawKeys(p, pixelsPerFrame, drawY,
                          startFrame, endFrame,
                          rowHeight, keyRectSize);
    ComplexAnimator::prp_drawKeys(p, pixelsPerFrame, drawY,
                                  startFrame, endFrame,
                                  rowHeight, keyRectSize);
}

Key *FakeComplexAnimator::prp_getKeyAtPos(const qreal &relX,
                                          const int &minViewedFrame,
                                          const qreal &pixelsPerFrame,
                                          const int &keyRectSize) {
    Key *key = ComplexAnimator::prp_getKeyAtPos(relX, minViewedFrame,
                                                pixelsPerFrame,
                                                keyRectSize);
    if(key != nullptr) return key;
    return mTarget->prp_getKeyAtPos(relX, minViewedFrame,
                                    pixelsPerFrame, keyRectSize);
}

void FakeComplexAnimator::prp_getKeysInRect(const QRectF &selectionRect,
                                            const qreal &pixelsPerFrame,
                                            QList<Key *> &keysList,
                                            const int &keyRectSize) {
    mTarget->prp_getKeysInRect(selectionRect, pixelsPerFrame,
                               keysList, keyRectSize);
    ComplexAnimator::prp_getKeysInRect(selectionRect, pixelsPerFrame,
                                       keysList, keyRectSize);
}

bool FakeComplexAnimator::SWT_isFakeComplexAnimator() { return true; }
