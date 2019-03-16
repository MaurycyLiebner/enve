#include "videobox.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include "GUI/mainwindow.h"
#include <QDebug>
#include "Sound/singlesound.h"
#include "canvas.h"
#include "Sound/soundcomposition.h"
#include "filesourcescache.h"
#include "FileCacheHandlers/videocachehandler.h"

VideoBox::VideoBox() : AnimationBox() {
    mType = TYPE_VIDEO;
    setName("Video");
}

VideoBox::VideoBox(const QString &filePath) :
    VideoBox() {
    setFilePath(filePath);
}

VideoBox::~VideoBox() {
    auto parentCanvas = getParentCanvas();
    if(parentCanvas && mSound) {
        parentCanvas->getSoundComposition()->removeSound(mSound);
    }
}

void VideoBox::setParentGroup(BoxesGroup *parent) {
    if(mParentGroup && mSound) {
        getParentCanvas()->getSoundComposition()->removeSound(mSound);
    }
    AnimationBox::setParentGroup(parent);
    if(mParentGroup && mSound) {
        getParentCanvas()->getSoundComposition()->addSound(mSound);
    }
}

#include <QFileDialog>
void VideoBox::changeSourceFile(QWidget *dialogParent) {
    const QString importPath = QFileDialog::getOpenFileName(
                dialogParent, "Change Source", "",
                "Video Files (*.mp4 *.mov *.avi *.mkv *.m4v)");
    if(!importPath.isEmpty()) setFilePath(importPath);
}

FrameRange VideoBox::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    if(isRelFrameVisibleAndInVisibleDurationRect(relFrame)) {
        return {relFrame, relFrame};
    }
    return BoundingBox::prp_getIdenticalRelFrameRange(relFrame);
}

void VideoBox::setFilePath(const QString &path) {
    mSrcFilePath = path;
    if(!mAnimationCacheHandler) {
        auto currentHandler =
                FileSourcesCache::getHandlerForFilePath(mSrcFilePath);
        mAnimationCacheHandler =
                GetAsPtr(currentHandler, AnimationCacheHandler);
        if(!mAnimationCacheHandler) {
            auto newHandler = FileSourcesCache::
                    createNewHandler<VideoCacheHandler>(
                        mSrcFilePath);
            mAnimationCacheHandler = newHandler;
        }
        mAnimationCacheHandler->addDependentBox(this);
        //getAnimationDurationRect()->setCacheHandler(mAnimationCacheHandler);
    }
    reloadCacheHandler();
}

bool hasSound(const char* path) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if(avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return false;
    }
    if(avformat_find_stream_info(format, nullptr) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return false;
    }

    // Find the index of the first audio stream
    for(uint i = 0; i < format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            return true;
        }
    }

    avformat_free_context(format);

    // success
    return false;
}

void VideoBox::reloadSound() {
    if(hasSound(mSrcFilePath.toLatin1().data())) {
        if(mSound) {
            mSound->reloadDataFromFile();
        } else {
            auto flar = GetAsPtr(mDurationRectangle, FixedLenAnimationRect);
            mSound = SPtrCreate(SingleSound)(mSrcFilePath, flar);
            ca_addChildAnimator(mSound);
            if(mParentGroup) {
                getParentCanvas()->getSoundComposition()->addSound(mSound);
            }
        }
    }
}
