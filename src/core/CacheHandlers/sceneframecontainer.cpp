#include "sceneframecontainer.h"
#include "../Boxes/boxrenderdata.h"
#include "../canvas.h"

SceneFrameContainer::SceneFrameContainer(
        Canvas * const scene,
        const BoxRenderData * const data,
        const FrameRange &range,
        HddCachableCacheHandler * const parent) :
    ImageCacheContainer(data->fRenderedImage, range, parent),
    fBoxState(data->fBoxStateId), mScene(scene) {}

stdsptr<eHddTask> SceneFrameContainer::createTmpFileDataSaver() {
    const ImgTmpFileDataSaver::Func func = [this](qsptr<QTemporaryFile> tmpFile) {
        setDataSavedToTmpFile(tmpFile);
    };
    return enve::make_shared<ImgTmpFileDataSaver>(mImageSk, func);
}

stdsptr<eHddTask> SceneFrameContainer::createTmpFileDataLoader() {
    const ImgTmpFileDataLoader::Func func = [this](sk_sp<SkImage> img) {
        setDataLoadedFromTmpFile(img);
        if(mScene) mScene->setSceneFrame(ref<SceneFrameContainer>());
    };
    return enve::make_shared<ImgTmpFileDataLoader>(mTmpFile, func);
}
