#include "videobox.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include "mainwindow.h"
#include <QDebug>
#include "Sound/singlesound.h"
#include "canvas.h"
#include "Sound/soundcomposition.h"
#include "filesourcescache.h"

VideoBox::VideoBox() :
    AnimationBox() {
    mType = TYPE_VIDEO;
    setName("Video");
}

VideoBox::VideoBox(const QString &filePath) :
    VideoBox() {
    setFilePath(filePath);
}

void VideoBox::setParentGroup(BoxesGroup *parent) {
    if(mParentGroup != NULL && mSound != NULL) {
        getParentCanvas()->getSoundComposition()->removeSound(mSound);
    }
    AnimationBox::setParentGroup(parent);
    if(mParentGroup != NULL && mSound != NULL) {
        getParentCanvas()->getSoundComposition()->addSound(mSound);
    }
}

#include <QFileDialog>
void VideoBox::changeSourceFile() {
    MainWindow::getInstance()->disableEventFilter();
    QString importPath = QFileDialog::getOpenFileName(
                                            MainWindow::getInstance(),
                                            "Change Source", "",
                                            "Video Files (*.mp4 *.mov *.avi)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPath.isEmpty()) {
        setFilePath(importPath);
    }
}

void VideoBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                     int *lastIdentical,
                                                     const int &relFrame) {
    if(isRelFrameVisibleAndInVisibleDurationRect(relFrame)) {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    } else {
        BoundingBox::prp_getFirstAndLastIdenticalRelFrame(firstIdentical,
                                                           lastIdentical,
                                                           relFrame);
    }
}

void VideoBox::setFilePath(const QString &path) {
    mSrcFilePath = path;
    if(mAnimationCacheHandler == NULL) {
        mAnimationCacheHandler = (AnimationCacheHandler*)
                                    FileSourcesCache::getHandlerForFilePath(
                                                            mSrcFilePath);
        if(mAnimationCacheHandler == NULL) {
            mAnimationCacheHandler = new VideoCacheHandler(mSrcFilePath);
        }
        mAnimationCacheHandler->addDependentBox(this);
    }
    reloadCacheHandler();
}

bool hasSound(const char* path) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return false;
    }
    if(avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return false;
    }

    // Find the index of the first audio stream
    for (uint i = 0; i < format->nb_streams; i++) {
        const AVMediaType &mediaType = format->streams[i]->codec->codec_type;
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
        if(mSound == NULL) {
            mSound = new SingleSound(mSrcFilePath,
                                     (FixedLenAnimationRect*)mDurationRectangle);
            ca_addChildAnimator(mSound);
            if(mParentGroup != NULL) {
                getParentCanvas()->getSoundComposition()->addSound(mSound);
            }
        } else {
            mSound->reloadDataFromFile();
        }
    } else {
    }
}
