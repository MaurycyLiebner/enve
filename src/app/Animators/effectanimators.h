#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "Animators/dynamiccomplexanimator.h"
#include "smartPointers/sharedpointerdefs.h"
#include "PixmapEffects/pixmapeffect.h"

class BoundingBox;
class SkImage;
class SkCanvas;
class SkBitmap;
struct PixmapEffectRenderData;

qsptr<PixmapEffect> readIdCreatePixmapEffect(QIODevice * const src);

typedef DynamicComplexAnimator<PixmapEffect,
        &PixmapEffect::writeIdentifier,
        &readIdCreatePixmapEffect> EffectAnimatorsBase;
class EffectAnimators : public EffectAnimatorsBase {
    friend class SelfRef;
protected:
    EffectAnimators(BoundingBox *parentBox);
public:
    bool SWT_isPixmapEffectAnimators() const { return true; }

    void readPixmapEffect(QIODevice * const src);

    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() {
        return mParentBox_k;
    }

    bool hasEffects();

    QMarginsF getEffectsMargin(const qreal relFrame) const;

    void addEffects(const qreal relFrame,
                    BoxRenderData * const data);
private:
    BoundingBox * const mParentBox_k;
};

#endif // EFFECTANIMATORS_H
