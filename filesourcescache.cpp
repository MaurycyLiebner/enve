#include "filesourcescache.h"
#include "Boxes/rendercachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "Boxes/boundingbox.h"
#include "filesourcelist.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

QList<std::shared_ptr<FileCacheHandler> >
FileSourcesCache::mFileCacheHandlers;
QList<FileSourceListVisibleWidget*>
FileSourcesCache::mFileSourceListVisibleWidgets;

FileSourcesCache::FileSourcesCache() {

}

void FileSourcesCache::addFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    mFileSourceListVisibleWidgets << wid;
}

void FileSourcesCache::removeFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    mFileSourceListVisibleWidgets.removeOne(wid);
}

void FileSourcesCache::addHandlerToHandlersList(FileCacheHandler *handlerPtr) {
    mFileCacheHandlers.append(handlerPtr->ref<FileCacheHandler>());
}

void FileSourcesCache::addHandlerToListWidgets(FileCacheHandler *handlerPtr) {
    foreach(FileSourceListVisibleWidget *wid, mFileSourceListVisibleWidgets) {
        wid->addCacheHandlerToList(handlerPtr);
    }
}

FileCacheHandler *FileSourcesCache::getHandlerForFilePath(
        const QString &filePath) {
    Q_FOREACH(const std::shared_ptr<FileCacheHandler> &handler,
              mFileCacheHandlers) {
        if(handler->getFilePath() == filePath) {
            return handler.get();
        }
    }
    QString ext = filePath.split(".").last();
    if(isVideoExt(ext)) {
        return new VideoCacheHandler(filePath);
    } else if(isImageExt(ext)) {
        return new ImageCacheHandler(filePath);
    }
    return NULL;
}

void FileSourcesCache::removeHandler(FileCacheHandler *handler) {
    for(int i = 0 ; i < mFileCacheHandlers.count(); i++) {
        if(mFileCacheHandlers.at(i).get() == handler) {
            mFileCacheHandlers.removeAt(i);
            break;
        }
    }
    foreach(FileSourceListVisibleWidget *wid, mFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handler);
    }
}

void FileSourcesCache::removeHandlerFromListWidgets(FileCacheHandler *handlerPtr) {
    foreach(FileSourceListVisibleWidget *wid, mFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handlerPtr);
    }
}

void FileSourcesCache::clearAll() {
    Q_FOREACH(const std::shared_ptr<FileCacheHandler> &handler,
              mFileCacheHandlers) {
        handler->clearCache();
    }
    mFileCacheHandlers.clear();
}

int FileSourcesCache::getFileCacheListCount() {
    return mFileCacheHandlers.count();
}

FileCacheHandler::FileCacheHandler(const QString &filePath,
                                   const bool &visibleInListWidgets) {
    mVisibleInListWidgets = visibleInListWidgets;
    mFileHandlerRef = ref<FileCacheHandler>();
    mFilePath = filePath;
    FileSourcesCache::addHandlerToHandlersList(this);
    if(visibleInListWidgets) {
        FileSourcesCache::addHandlerToListWidgets(this);
    }
}

void FileCacheHandler::setVisibleInListWidgets(const bool &bT) {
    if(bT == mVisibleInListWidgets) return;
    mVisibleInListWidgets = bT;
    if(bT) {
        FileSourcesCache::addHandlerToListWidgets(this);
    } else {
        FileSourcesCache::removeHandlerFromListWidgets(this);
    }
}

FileCacheHandler::~FileCacheHandler() {
    FileSourcesCache::removeHandler(this);
}

void FileCacheHandler::clearCache() {
    foreach(const BoundingBoxQSPtr &boxPtr, mDependentBoxes) {
        boxPtr->reloadCacheHandler();
    }
}

void FileCacheHandler::addDependentBox(BoundingBox *dependent) {
    mDependentBoxes << dependent->ref<BoundingBox>();
}

void FileCacheHandler::removeDependentBox(BoundingBox *dependent) {
    for(int i = 0; i < mDependentBoxes.count(); i++) {
        const BoundingBoxQSPtr &boxPtr = mDependentBoxes.at(i);
        if(boxPtr.data() == dependent) {
            mDependentBoxes.removeAt(i);
            return;
        }
    }
}

ImageCacheHandler::ImageCacheHandler(const QString &filePath,
                                     const bool &visibleSeparatly) :
    FileCacheHandler(filePath) {
    mVisibleInListWidgets = visibleSeparatly;
}

void ImageCacheHandler::processUpdate() {
    sk_sp<SkData> data = SkData::MakeFromFileName(
                mFilePath.toLocal8Bit().data());
    mUpdateImage = SkImage::MakeFromEncoded(data);
}

void ImageCacheHandler::afterUpdate() {
    FileCacheHandler::afterUpdate();
    mImage = mUpdateImage;
    mUpdateImage.reset();
}

VideoCacheHandler::VideoCacheHandler(const QString &filePath) :
    AnimationCacheHandler(filePath) {
    updateFrameCount();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtFrame(const int &relFrame) {
    CacheContainer *cont = mFramesCache.getRenderContainerAtRelFrame(relFrame);
    if(cont == NULL) return sk_sp<SkImage>();
    cont->neededInMemory();
    return cont->getImageSk();
}

void VideoCacheHandler::beforeUpdate() {
    FileCacheHandler::beforeUpdate();
    qDebug() << "loading: " << mFramesLoadScheduled;
    mFramesBeingLoaded = mFramesLoadScheduled;
    mFramesBeingLoadedGUI = mFramesBeingLoaded;
    mFramesLoadScheduled.clear();
    mUpdateFilePath = mFilePath;
    qSort(mFramesBeingLoaded);
}

void VideoCacheHandler::updateFrameCount() {
    QByteArray stringByteArray = mFilePath.toLatin1();
    const char *path = stringByteArray.constData();
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return;
    }

    // Find the index of the first audio stream
    for (uint i = 0; i < format->nb_streams; i++) {
        const AVMediaType &mediaType = format->streams[i]->codec->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            AVStream *vidStream = format->streams[i];
            mFps = (qreal)vidStream->avg_frame_rate.num/
                        vidStream->avg_frame_rate.den;
            mFramesCount = vidStream->nb_frames - 1;
            break;
        }
    }

    avformat_free_context(format);
}
#include <QElapsedTimer>
void VideoCacheHandler::processUpdate() {
    QElapsedTimer timer;
    timer.start();
    QByteArray pathByteArray = mUpdateFilePath.toLatin1();
    const char* path = pathByteArray.data();
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return;// -1;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr,
                "Could not retrieve stream info from file '%s'\n",
                path);
        return;// -1;
    }

    // Find the index of the first audio stream
    int videoStreamIndex = -1;
    for (uint i = 0; i < format->nb_streams; i++) {
        const AVMediaType &mediaType =
                format->streams[i]->codec->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if(videoStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve video stream from file '%s'\n",
                path);
        return;// -1;
    }
    AVCodecContext *videoCodec = NULL;
    struct SwsContext *sws = NULL;

    AVStream *videoStream = format->streams[videoStreamIndex];
    videoCodec = videoStream->codec;
    if( avcodec_open2(videoCodec,
                      avcodec_find_decoder(videoCodec->codec_id),
                      NULL) < 0 ) {
        fprintf(stderr,
                "Failed to open decoder for stream #%u in file '%s'\n",
                videoStreamIndex, path);
        return;// -1;
    }

    sws = sws_getContext(videoCodec->width, videoCodec->height,
                         videoCodec->pix_fmt,
                         videoCodec->width, videoCodec->height,
                         AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame *decodedFrame = av_frame_alloc();
    if(!decodedFrame) {
        fprintf(stderr, "Error allocating the frame\n");
        return;// -1;
    }

    QElapsedTimer framesTimer;
    framesTimer.start();
    foreach(const int &frameId, mFramesBeingLoaded) {
        int tsms = qRound(frameId * 1000 /
                          (double)videoStream->avg_frame_rate.num*
                          videoStream->avg_frame_rate.den);

        int64_t frame = av_rescale(tsms, videoStream->time_base.den,
                                   videoStream->time_base.num);

        frame /= 1000;

        if(frameId != 0) {
            if(avformat_seek_file(format, videoStreamIndex, 0,
                                   frame, frame,
                    AVSEEK_FLAG_FRAME) < 0) {
                return;// 0;
            }
        }

        avcodec_flush_buffers(videoCodec);

        int64_t pts = 0;

        do {
            if(av_read_frame(format, &packet) >= 0) {
                int gotFrame;
                if(packet.stream_index == videoStreamIndex) {
                    avcodec_decode_video2(videoCodec, decodedFrame, &gotFrame,
                    &packet);
                }
                av_free_packet(&packet);
                if(frameId == 0 && gotFrame) break;
            } else {
                break;
            }

            // calculate PTS:
            pts = av_frame_get_best_effort_timestamp(decodedFrame);
            pts = av_rescale_q(pts, videoStream->time_base, AV_TIME_BASE_Q);
        } while(pts/1000 <= tsms);

    // SKIA

        SkImageInfo info = SkImageInfo::Make(videoCodec->width,
                                             videoCodec->height,
                                             kBGRA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr);
        SkBitmap bitmap;
        bitmap.allocPixels(info);

        SkPixmap pixmap;
        bitmap.peekPixels(&pixmap);

        uint8_t *dstSk[] = {(unsigned char*)pixmap.writable_addr()};
        int linesizesSk[4];

        av_image_fill_linesizes(linesizesSk,
                                AV_PIX_FMT_BGRA,
                                decodedFrame->width);

        sws_scale(sws, decodedFrame->data, decodedFrame->linesize,
                  0, videoCodec->height,
                  dstSk, linesizesSk);

        mLoadedFrames << SkImage::MakeFromBitmap(bitmap);
    // SKIA
    }
    qDebug() << "elapsed loading " <<
                mFramesBeingLoaded.count() <<
                " frames: " << framesTimer.elapsed();

    // clean up
    av_frame_free(&decodedFrame);

    sws_freeContext(sws);
    avcodec_close(videoCodec);

    avformat_free_context(format);

    qDebug() << "total elapsed: " << timer.elapsed();
    // success
    return;// 0;
}

void VideoCacheHandler::afterUpdate() {
    FileCacheHandler::afterUpdate();
    qDebug() << "loaded: " << mFramesBeingLoaded;
    for(int i = 0; i < mFramesBeingLoaded.count(); i++) {
        CacheContainer *cont =
                mFramesCache.createNewRenderContainerAtRelFrame(
                    mFramesBeingLoaded.at(i));
        cont->replaceImageSk(mLoadedFrames.at(i));
    }
    mLoadedFrames.clear();
    mFramesBeingLoaded.clear();
    mFramesBeingLoadedGUI.clear();
}

void VideoCacheHandler::clearCache() {
    mFramesCache.clearCache();
    AnimationCacheHandler::clearCache();
}

const qreal &VideoCacheHandler::getFps() { return mFps; }

Updatable *VideoCacheHandler::scheduleFrameLoad(const int &frame) {
    if(mFramesLoadScheduled.contains(frame) ||
       mFramesBeingLoadedGUI.contains(frame)) return this;
    qDebug() << "schedule frame load: " << frame;
    mFramesLoadScheduled << frame;
    addScheduler();
    return this;
}

ImageSequenceCacheHandler::ImageSequenceCacheHandler(
        const QStringList &framePaths) {
    mFramePaths = framePaths;
    foreach(const QString &path, framePaths) {
        ImageCacheHandler *imgCacheHandler = (ImageCacheHandler*)
                FileSourcesCache::getHandlerForFilePath(path);
        if(imgCacheHandler == NULL) {
            mFrameImageHandlers << (new ImageCacheHandler(path, false))->ref<ImageCacheHandler>();
        } else {
            mFrameImageHandlers << imgCacheHandler->ref<ImageCacheHandler>();
        }
    }
    updateFrameCount();
}

sk_sp<SkImage> ImageSequenceCacheHandler::getFrameAtFrame(const int &relFrame) {
    ImageCacheHandler *cacheHandler = mFrameImageHandlers.at(relFrame).get();
    if(cacheHandler == NULL) return sk_sp<SkImage>();
    return cacheHandler->getImage();
}

void ImageSequenceCacheHandler::updateFrameCount() {
    mFramesCount = mFramePaths.count();
}

void ImageSequenceCacheHandler::clearCache() {
    foreach(const std::shared_ptr<ImageCacheHandler> &cacheHandler, mFrameImageHandlers) {
        cacheHandler->clearCache();
    }
    FileCacheHandler::clearCache();
}

Updatable *ImageSequenceCacheHandler::scheduleFrameLoad(const int &frame) {
    ImageCacheHandler *imageHandler = mFrameImageHandlers.at(frame).get();
    imageHandler->addScheduler();
    return imageHandler;
}

bool isVideoExt(const QString &extension) {
    return extension == "avi" ||
           extension == "mp4" ||
           extension == "mov";
}

bool isSoundExt(const QString &extension) {
    return extension == "mp3" ||
           extension == "wav";
}

bool isVectorExt(const QString &extension) {
    return extension == "svg";
}

bool isImageExt(const QString &extension) {
    return extension == "png" ||
           extension == "jpg";
}

bool isAvExt(const QString &extension) {
    return extension == "av";
}
