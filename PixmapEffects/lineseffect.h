#ifndef LINESEFFECT_H
#define LINESEFFECT_H
#include "pixmapeffect.h"

struct LinesEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool vertical = false;
    qreal linesDistance;
    qreal linesWidth;
protected:
    LinesEffectRenderData() {}
};

class LinesEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    LinesEffect(qreal linesWidth = 5.,
                qreal linesDistance = 5.);
private:
    bool mVertical = false;
    QrealAnimatorQSPtr mLinesDistance;
    QrealAnimatorQSPtr mLinesWidth;
};
#endif // LINESEFFECT_H
