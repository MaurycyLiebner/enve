#ifndef FAKECOMPLEXANIMATOR_H
#define FAKECOMPLEXANIMATOR_H
#include "complexanimator.h"

class FakeComplexAnimator : public ComplexAnimator {
    friend class SelfRef;
protected:
    FakeComplexAnimator(const QString& name, Property *target);
public:
    Property *getTarget();

    void drawTimelineControls(QPainter * const p,
                              const qreal pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int rowHeight);

    Key *anim_getKeyAtPos(const qreal relX,
                          const int minViewedFrame,
                          const qreal pixelsPerFrame,
                          const int keyRectSize);

    void anim_getKeysInRect(const QRectF &selectionRect,
                           const qreal pixelsPerFrame,
                           QList<Key *>& keysList,
                           const int keyRectSize);

    bool SWT_isFakeComplexAnimator() const;

    using ComplexAnimator::ca_addChild;
    using ComplexAnimator::ca_insertChild;
    using ComplexAnimator::ca_removeChild;
    using ComplexAnimator::ca_takeChildAt;
    using ComplexAnimator::ca_prependChildAnimator;
    using ComplexAnimator::ca_replaceChildAnimator;
private:
    Property *mTarget = nullptr;
};

#endif // FAKECOMPLEXANIMATOR_H
