#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "complexanimator.h"

namespace fmt_filters { struct image; }

class PixmapEffect;
class BoundingBox;
class SkImage;
class SkCanvas;
class SkBitmap;
struct PixmapEffectRenderData;

class EffectAnimators : public ComplexAnimator {
public:
    EffectAnimators();

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

    void makeDuplicate(Property *target);
    Property *makeDuplicate() {
        return NULL;
    }

    bool SWT_isEffectAnimators() { return true; }
    qreal getEffectsMarginAtRelFrame(const int &relFrame) const;

    void addEffectRenderDataToList(const int &relFrame,
            QList<PixmapEffectRenderData*> *pixmapEffects);

    void writeEffectAnimators(std::fstream *file);
    void readEffectAnimators(std::fstream *file);
private:
    BoundingBox *mParentBox;
};

#endif // EFFECTANIMATORS_H
