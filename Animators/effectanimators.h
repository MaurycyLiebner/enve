#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "complexanimator.h"

namespace fmt_filters { struct image; }

class PixmapEffect;
class BoundingBox;
class SkImage;
class SkCanvas;
class SkBitmap;
class BoundingBoxRenderData;
struct PixmapEffectRenderData;

class EffectAnimators : public ComplexAnimator {
public:
    EffectAnimators(BoundingBox *parentBox);

    void addEffect(PixmapEffect *effect);

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal getEffectsMargin() const;

    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() {
        return mParentBox;
    }

    bool hasEffects();

    bool SWT_isPixmapEffectAnimators() { return true; }
    qreal getEffectsMarginAtRelFrame(const int &relFrame) const;
    qreal getEffectsMarginAtRelFrameF(const qreal &relFrame) const;

    void addEffectRenderDataToList(const int &relFrame,
                                   BoundingBoxRenderData *data);
    void addEffectRenderDataToListF(const qreal &relFrame,
                                    BoundingBoxRenderData *data);

    void ca_removeAllChildAnimators();

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
    void readPixmapEffect(QIODevice *target);
private:
    BoundingBox *mParentBox = nullptr;
};

#endif // EFFECTANIMATORS_H
