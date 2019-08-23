#include "effectsubtaskspawner.h"
#include "boxrenderdata.h"
#include "Tasks/taskscheduler.h"
#include "skia/skiaincludes.h"
#include "RasterEffects/rastereffect.h"
#include "RasterEffects/rastereffectcaller.h"

class EffectSubTaskSpawner_priv {
public:
    EffectSubTaskSpawner_priv(const stdsptr<RasterEffectCaller>& effect,
                              const stdsptr<BoxRenderData>& data);
private:
    void decRemaining_k();
    void spawn();
    void splitSpawn(CpuRenderData& data,
                    const SkIRect& rect,
                    const int nSplits);

    int mRemaining = 0;
    const stdsptr<RasterEffectCaller> mEffectCaller;
    const stdsptr<BoxRenderData> mData;
    SkBitmap mSrcBitmap;
    SkBitmap mDstBitmap;
};

EffectSubTaskSpawner_priv::EffectSubTaskSpawner_priv(
        const stdsptr<RasterEffectCaller> &effect,
        const stdsptr<BoxRenderData> &data) :
    mEffectCaller(effect), mData(data) {
    SkPixmap pixmap;
    data->fRenderedImage->peekPixels(&pixmap);
    mSrcBitmap.installPixels(pixmap);
    mDstBitmap.allocPixels(mSrcBitmap.info());
    spawn();
}

void EffectSubTaskSpawner_priv::splitSpawn(CpuRenderData& data,
                                           const SkIRect& rect,
                                           const int nSplits) {
    if(nSplits == 0) return;
    if(nSplits == 1) {
        data.fTexTile = rect;
        const auto subTask = enve::make_shared<eCustomCpuTask>(nullptr,
            [this, data]() {
                CpuRenderTools tools(mSrcBitmap, mDstBitmap);
                mEffectCaller->processCpu(tools, data);
            }, [this]() { decRemaining_k(); });
        TaskScheduler::sGetInstance()->scheduleCpuTask(subTask);
        return;
    }

    const int splits1 = nSplits/2;
    const int splits2 = nSplits - splits1;
    const bool splitWidth = rect.width() > rect.height();
    if(splitWidth) {
        const int width1 = rect.width()*splits1/nSplits;
        const auto rect1 = SkIRect::MakeXYWH(rect.x(), rect.y(),
                                             width1, rect.height());
        splitSpawn(data, rect1, splits1);

        //const int width2 = rect.width() - width1;
        const auto rect2 = SkIRect::MakeLTRB(rect1.right(), rect.top(),
                                             rect.right(), rect.bottom());
        splitSpawn(data, rect2, splits2);
    } else {
        const int height1 = rect.height()*splits1/nSplits;
        const auto rect1 = SkIRect::MakeXYWH(rect.x(), rect.y(),
                                             rect.width(), height1);
        splitSpawn(data, rect1, splits1);

        //const int height2 = rect.height() - height1;
        const auto rect2 = SkIRect::MakeLTRB(rect.left(), rect1.bottom(),
                                             rect.right(), rect.bottom());
        splitSpawn(data, rect2, splits2);
    }
}

void EffectSubTaskSpawner_priv::spawn() {
    const int width = mSrcBitmap.width();
    const int height = mSrcBitmap.height();
    const int area = width*height;
    const int nAllThreads = QThread::idealThreadCount();
    const int nThreads = qMax(1, mEffectCaller->cpuThreads(nAllThreads, area));
    mRemaining = nThreads;

    auto& srcImage = mData->fRenderedImage;
    const int srcWidth = srcImage->width();
    const int srcHeight = srcImage->height();
    const QPoint gPos = mData->fGlobalRect.topLeft();
    CpuRenderData data;
    data.fPosX = static_cast<int>(gPos.x());
    data.fPosY = static_cast<int>(gPos.y());
    data.fWidth = static_cast<uint>(srcWidth);
    data.fHeight = static_cast<uint>(srcHeight);

    splitSpawn(data, srcImage->bounds(), nThreads);
}

void EffectSubTaskSpawner_priv::decRemaining_k() {
    if(--mRemaining > 0) return;
    if(mData->getState() != eTaskState::canceled) {
        mData->fRenderedImage = SkiaHelpers::transferDataToSkImage(mDstBitmap);
        if(mData->nextStep()) {
            TaskScheduler::sGetInstance()->queCpuTaskFastTrack(mData);
        } else mData->finishedProcessing();
    }
    delete this;
}


void EffectSubTaskSpawner::sSpawn(const stdsptr<RasterEffectCaller> &effect,
                                  const stdsptr<BoxRenderData> &data) {
    new EffectSubTaskSpawner_priv(effect, data);
}
