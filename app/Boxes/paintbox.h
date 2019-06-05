#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
#include "Paint/animatedsurface.h"
#include "imagebox.h"
class QPointFAnimator;
class AnimatedPoint;
class SimpleBrushWrapper;

struct PaintBoxRenderData : public ImageRenderData {
    friend class StdSelfRef;
    PaintBoxRenderData(BoundingBox * const parentBoxT) :
        ImageRenderData(parentBoxT) {}

    void loadImageFromHandler() {
        auto bitmap = fSurface->surface().toBitmap();
        fImage = SkiaHelpers::transferDataToSkImage(bitmap);
    }

    void updateRelBoundingRect() final {
        fRelBoundingRect = fSurface->pixelBoundingRect();
    }

    stdsptr<DrawableAutoTiledSurface> fSurface;
};

class PaintBox : public BoundingBox {
    friend class SelfRef;
protected:
    PaintBox();
public:
    bool SWT_isPaintBox() const { return true; }
    void setupRenderData(const qreal relFrame,
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
