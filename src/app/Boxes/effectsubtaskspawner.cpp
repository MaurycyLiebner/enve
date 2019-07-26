#include "effectsubtaskspawner.h"
#include "boxrenderdata.h"
#include "taskscheduler.h"

void EffectSubTaskSpawner::sSpawn(const stdsptr<RasterEffectCaller> &effect,
                                  const stdsptr<BoxRenderData> &data) {
    new EffectSubTaskSpawner(effect, data);
}

EffectSubTaskSpawner::EffectSubTaskSpawner(
        const stdsptr<RasterEffectCaller> &effect,
        const stdsptr<BoxRenderData> &data) :
    mEffectCaller(effect), mData(data) {
    SkPixmap pixmap;
    data->fRenderedImage->peekPixels(&pixmap);
    mSrcBitmap.installPixels(pixmap);
    mDstBitmap.allocPixels(mSrcBitmap.info());
    spawn();
}

void EffectSubTaskSpawner::spawn() {
    const int hDiv = 2;
    const int vDiv = 2;
    const int width = mSrcBitmap.width();
    const int height = mSrcBitmap.height();
    const qreal wFrag = 1.*width/hDiv;
    const qreal hFrag = 1.*height/vDiv;

    auto& srcImage = mData->fRenderedImage;
    const int srcWidth = srcImage->width();
    const int srcHeight = srcImage->height();
    const QPoint gPos = mData->fGlobalRect.topLeft();
    CpuRenderData data;
    data.fPosX = static_cast<int>(gPos.x());
    data.fPosY = static_cast<int>(gPos.y());
    data.fWidth = static_cast<uint>(srcWidth);
    data.fHeight = static_cast<uint>(srcHeight);
    mRemaining = hDiv*vDiv;
    for(int i = 0; i < hDiv; i++) {
        for(int j = 0; j < vDiv; j++) {
            const SkIRect tileRect = SkIRect::MakeXYWH(
                        qRound(i*wFrag), qRound(j*hFrag),
                        qRound((i + 1)*wFrag), qRound((j + 1)*hFrag));
            data.fTexTile = tileRect;
            const auto subTask = SPtrCreate(CustomCPUTask)(nullptr,
                [this, data]() {
                    CpuRenderTools tools(mSrcBitmap, mDstBitmap);
                    mEffectCaller->processCpu(tools, data);
                }, [this]() { decRemaining_k(); });
            TaskScheduler::sGetInstance()->scheduleCPUTask(subTask);
        }
    }
}

void EffectSubTaskSpawner::decRemaining_k() {
    if(--mRemaining > 0) return;
    if(mData->getState() != Task::CANCELED) {
        mData->fRenderedImage = SkiaHelpers::transferDataToSkImage(mDstBitmap);
        if(mData->nextStep()) {
            TaskScheduler::sGetInstance()->scheduleCPUTask(mData);
        } else mData->finishedProcessing();
    }
    delete this;
}
