#include "videobox.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <QDebug>
#include "Sound/singlesound.h"
#include "canvas.h"
#include "Sound/soundcomposition.h"
#include "filesourcescache.h"
#include "fileshandler.h"

VideoBox::VideoBox() : AnimationBox(TYPE_VIDEO) {
    prp_setName("Video");
    const auto flar = mDurationRectangle->ref<FixedLenAnimationRect>();
    mSound = enve::make_shared<SingleSound>(flar);
    ca_addChild(mSound);
    mSound->SWT_hide();

    connect(this, &eBoxOrSound::parentChanged,
            mSound.get(), &eBoxOrSound::setParentGroup);
}

void VideoBox::writeBoundingBox(QIODevice * const target) {
    AnimationBox::writeBoundingBox(target);

}

void VideoBox::readBoundingBox(QIODevice * const target) {
    AnimationBox::readBoundingBox(target);

}

#include <QFileDialog>
void VideoBox::changeSourceFile(QWidget * const dialogParent) {
    const QString path = QFileDialog::getOpenFileName(
                dialogParent, "Change Source", "",
                "Video Files (*.mp4 *.mov *.avi *.mkv *.m4v)");
    if(!path.isEmpty()) setFilePath(path);
}

void VideoBox::setStretch(const qreal stretch) {
    AnimationBox::setStretch(stretch);
    mSound->setStretch(stretch);
}

void VideoBox::setFilePath(const QString &path) {
    if(mSrcFramesCache) {
        const auto videoSrc = static_cast<VideoFrameHandler*>(mSrcFramesCache.get());
        const auto oldDataHandler = videoSrc->getDataHandler();
        disconnect(mFileHandler, &VideoFileHandler::pathChanged,
                   this, &VideoBox::animationDataChanged);
        disconnect(mFileHandler, &VideoFileHandler::reloaded,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        disconnect(oldDataHandler, &VideoDataHandler::frameCountUpdated,
                   this, &VideoBox::updateDurationRectangleAnimationRange);
    }
    mSrcFramesCache.reset();

    mFileHandler = FilesHandler::sInstance->getFileHandler<VideoFileHandler>(path);
    const auto newDataHandler = mFileHandler->getFrameHandler();
    if(newDataHandler) {
        mSrcFramesCache = enve::make_shared<VideoFrameHandler>(newDataHandler);
        getAnimationDurationRect()->setRasterCacheHandler(
                    &newDataHandler->getCacheHandler());
        connect(mFileHandler, &VideoFileHandler::pathChanged,
                this, &VideoBox::animationDataChanged);
        connect(mFileHandler, &VideoFileHandler::reloaded,
                this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        connect(newDataHandler, &VideoDataHandler::frameCountUpdated,
                this, &VideoBox::updateDurationRectangleAnimationRange);
    } else {
        getAnimationDurationRect()->setRasterCacheHandler(nullptr);
    }

    animationDataChanged();
}

void VideoBox::setSoundEnabled(const bool enable) {
    if(mSoundEnabled == enable) return;
    mSoundEnabled = enable;
    mSound->setEnabled(enable);
}

void VideoBox::animationDataChanged() {
    soundDataChanged();
    AnimationBox::animationDataChanged();
}

void VideoBox::soundDataChanged() {
    const auto soundHandler = mFileHandler->getSoundHandler();
    if(soundHandler) {
        if(!mSound->SWT_isVisible()) {
            if(mParentScene) {
                mParentScene->getSoundComposition()->addSound(mSound);
            }
        }
        mDurationRectangle->setSoundCacheHandler(&soundHandler->getCacheHandler());
    } else {
        if(mSound->SWT_isVisible()) {
            if(mParentScene) {
                mParentScene->getSoundComposition()->removeSound(mSound);
            }
        }
        mDurationRectangle->setSoundCacheHandler(nullptr);
    }
    mSound->setSoundDataHandler(soundHandler);
    mSound->SWT_setVisible(soundHandler);
}
