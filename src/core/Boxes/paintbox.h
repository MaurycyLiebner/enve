#ifndef PAINTBOX_H
#define PAINTBOX_H
#include "boundingbox.h"
#include "Paint/animatedsurface.h"
#include "imagebox.h"
class QPointFAnimator;
class AnimatedPoint;
class SimpleBrushWrapper;

struct PaintBoxRenderData : public ImageRenderData {
    e_OBJECT
public:
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
    e_OBJECT
protected:
    PaintBox();
public:
    bool SWT_isPaintBox() const { return true; }
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);
    stdsptr<BoxRenderData> createRenderData();

    void setupCanvasMenu(PropertyMenu * const menu);

    AnimatedSurface * getSurface() const {
        return mSurface.get();
    }

    void hideForPainting() {
        mVisible = false;
    }

    void showAfterPainting() {
        mVisible = true;
    }
private:
    qsptr<AnimatedSurface> mSurface;
};

#endif // PAINTBOX_H
