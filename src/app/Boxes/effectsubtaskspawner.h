#ifndef EFFECTSUBTASKSPAWNER_H
#define EFFECTSUBTASKSPAWNER_H
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"

struct BoxRenderData;
class RasterEffectCaller;

class EffectSubTaskSpawner {
public:
    EffectSubTaskSpawner(const stdsptr<RasterEffectCaller>& effect,
                         const stdsptr<BoxRenderData>& data);

    void spawn();
private:
    void decRemaining();

    const stdsptr<RasterEffectCaller> mEffectCaller;
    const stdsptr<BoxRenderData> mData;
    SkBitmap mSrcBitmap;
    SkBitmap mDstBitmap;
    int mRemaining = 0;
};

#endif // EFFECTSUBTASKSPAWNER_H
