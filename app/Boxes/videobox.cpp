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

VideoBox::VideoBox() : AnimationBox(TYPE_VIDEO) {
    setName("Video");
}

VideoBox::VideoBox(const QString &filePath) : VideoBox() {
    setFilePath(filePath);
}

VideoBox::~VideoBox() {
    auto parentCanvas = getParentCanvas();
    if(parentCanvas && mSound) {
        parentCanvas->getSoundComposition()->removeSound(mSound);
    }
}

void VideoBox::setParentGroup(BoxesGroup * const parent) {
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

void VideoBox::setFilePath(const QString &path) {
    mSrcFilePath = path;
    if(mSrcFramesCache) mSrcFramesCache->removeDependentBox(this);
    mSrcFramesCache.clear();

    auto currentHandler = FileSourcesCache::getHandlerForFilePath(mSrcFilePath);
    if(currentHandler) {
        mSrcFramesCache = GetAsPtr(currentHandler, AnimationCacheHandler);
        mSrcFramesCache->addDependentBox(this);
        const auto videoHandler = GetAsPtr(currentHandler, VideoCacheHandler);
        getAnimationDurationRect()->setCacheHandler(
                    &videoHandler->getCacheHandler());
    } else {
        getAnimationDurationRect()->setCacheHandler(nullptr);

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
        if(mSound) { // !!!
        } else {
            const auto flar = GetAsSPtr(mDurationRectangle,
                                        FixedLenAnimationRect);
            mSound = SPtrCreate(SingleSound)(flar);
            ca_addChildAnimator(mSound);
            mSound->setFilePath(mSrcFilePath);
            const auto parentCanvas = getParentCanvas();
            if(parentCanvas) {
                parentCanvas->getSoundComposition()->addSound(mSound);
            }
        }
    }
}
