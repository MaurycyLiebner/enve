#include "effectsubtaskspawner.h"
#include "boxrenderdata.h"
#include "taskscheduler.h"

EffectSubTaskSpawner::EffectSubTaskSpawner(
        const stdsptr<RasterEffectCaller> &effect,
        const stdsptr<BoxRenderData> &data) :
    mEffectCaller(effect), mData(data) {
    SkPixmap pixmap;
    data->fRenderedImage->peekPixels(&pixmap);
    mSrcBitmap.installPixels(pixmap);
    mDstBitmap.allocPixels(mSrcBitmap.info());
}

void EffectSubTaskSpawner::spawn() {
    const int hDiv = 2;
    const int vDiv = 2;
    const qreal hFrag = 1./hDiv;
    const qreal vFrag = 1./vDiv;
    const int width = mSrcBitmap.width();
    const int height = mSrcBitmap.height();
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
                        qRound(i*hFrag*width), qRound(j*vFrag*height),
                        qRound((i + 1)*hFrag*width), qRound((j + 1)*vFrag*height));
            data.fTexTile = tileRect;
            const auto subTask = SPtrCreate(CustomCPUTask)(nullptr,
                [this, data]() {
                    CpuRenderTools tools(mSrcBitmap, mDstBitmap);
                    mEffectCaller->processCpu(tools, data);
                }, [this]() { decRemaining(); });
            TaskScheduler::sGetInstance()->scheduleCPUTask(subTask);
        }
    }
}

void EffectSubTaskSpawner::decRemaining() {
    if(--mRemaining > 0) return;
    if(mData->getState() != Task::CANCELED) {
        mData->fRenderedImage = SkiaHelpers::transferDataToSkImage(mDstBitmap);
        if(mData->nextStep()) {
            TaskScheduler::sGetInstance()->scheduleCPUTask(mData);
        } else mData->finishedProcessing();
    }
}
