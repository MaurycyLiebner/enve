#ifndef SCENEFRAMECONTAINER_H
#define SCENEFRAMECONTAINER_H
#include "imagecachecontainer.h"
class BoxRenderData;

class SceneFrameContainer : public ImageCacheContainer {
public:
    SceneFrameContainer(Canvas * const scene,
                        const BoxRenderData* const data,
                        const FrameRange &range,
                        HddCachableCacheHandler * const parent);

    const uint fBoxState;
protected:
    stdsptr<eHddTask> createTmpFileDataSaver();
    stdsptr<eHddTask> createTmpFileDataLoader();
private:
    const qptr<Canvas> mScene;
};

#endif // SCENEFRAMECONTAINER_H
