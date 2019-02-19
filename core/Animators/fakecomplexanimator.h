#ifndef FAKECOMPLEXANIMATOR_H
#define FAKECOMPLEXANIMATOR_H
#include "complexanimator.h"

class FakeComplexAnimator : public ComplexAnimator {
    friend class SelfRef;
public:
    Property *getTarget();

    void anim_drawKeys(QPainter *p,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &endFrame,
                      const int &rowHeight,
                      const int &keyRectSize);

    Key *anim_getKeyAtPos(const qreal &relX,
                         const int &minViewedFrame,
                         const qreal &pixelsPerFrame,
                         const int& keyRectSize);

    void anim_getKeysInRect(const QRectF &selectionRect,
                           const qreal &pixelsPerFrame,
                           QList<Key *>& keysList,
                           const int& keyRectSize);

    bool SWT_isFakeComplexAnimator() const;
protected:
    FakeComplexAnimator(const QString& name, Property *target);
private:
    Property *mTarget = nullptr;
};

#endif // FAKECOMPLEXANIMATOR_H
