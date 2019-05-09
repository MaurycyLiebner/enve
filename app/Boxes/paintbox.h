#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
#include "Paint/animatedsurface.h"
class QPointFAnimator;
class AnimatedPoint;
class SimpleBrushWrapper;

struct PaintBoxRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    PaintBoxRenderData(BoundingBox * const parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {

    }

    void drawSk(SkCanvas * const canvas);
};

class PaintBox : public BoundingBox {
    friend class SelfRef;
protected:
    PaintBox();
public:
    bool SWT_isPaintBox() const { return true; }
    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
    stdsptr<BoundingBoxRenderData> createRenderData();

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void addActionsToMenu(BoxTypeMenu * const menu);

    AnimatedSurface * getSurface() const {
        return mSurface.get();
    }
private:
    qsptr<AnimatedSurface> mSurface;
};

#endif // PAINTBOX_H
