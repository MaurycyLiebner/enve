#include "filesourcescache.h"
#include "Boxes/rendercachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "Boxes/boundingbox.h"
#include "filesourcelist.h"
#include "Boxes/videobox.h"
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
    foreach(const BoundingBoxQWPtr &boxPtr, mDependentBoxes) {
        BoundingBox *box = boxPtr.data();
        if(box == NULL) continue;
        box->reloadCacheHandler();
    }
}

void FileCacheHandler::addDependentBox(BoundingBox *dependent) {
    mDependentBoxes << dependent->weakRef<BoundingBox>();
}

void FileCacheHandler::removeDependentBox(BoundingBox *dependent) {
    for(int i = 0; i < mDependentBoxes.count(); i++) {
        const BoundingBoxQWPtr &boxPtr = mDependentBoxes.at(i);
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
    //cont->neededInMemory();
    return cont->getImageSk();
}

void VideoCacheHandler::beforeUpdate() {
    FileCacheHandler::beforeUpdate();
    //qDebug() << "loading: " << mFramesLoadScheduled;
    mFramesBeingLoaded = mFramesLoadScheduled;
    mFramesBeingLoadedGUI = mFramesBeingLoaded;
    mFramesLoadScheduled.clear();
    mUpdateFilePath = mFilePath;
    qSort(mFramesBeingLoaded);
    mUpdateFps = mFps;
    mUpdateTimeBaseDen = mTimeBaseDen;
    mUpdateTimeBaseNum = mTimeBaseNum;
}

void VideoCacheHandler::updateFrameCount() {
    QByteArray stringByteArray = mFilePath.toLatin1();
    const char *path = stringByteArray.constData();
    AVFormatContext *format = avformat_alloc_context();
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
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            AVStream *vidStream = streamT;
            mTimeBaseDen = vidStream->avg_frame_rate.den;
            mTimeBaseNum = vidStream->avg_frame_rate.num;
            if(mTimeBaseDen != 0) {
                mFps = (qreal)mTimeBaseNum/mTimeBaseDen;
            }
            mFramesCount = vidStream->nb_frames;
            // try something else if retrieving frame count failed
            if(mFramesCount <= 0) {
                if(vidStream->r_frame_rate.den &&
                    vidStream->r_frame_rate.num) {
                    mTimeBaseDen = vidStream->r_frame_rate.den;
                    mTimeBaseNum = vidStream->r_frame_rate.num;
                    if(mTimeBaseDen == 0) {
                        mFramesCount = 0;
                        break;
                    } else {
                        mFps = mTimeBaseNum/(qreal)mTimeBaseDen;
                    }
                    int64_t duration = format->duration + (format->duration <= INT64_MAX - 5000 ? 5000 : 0);
                    mFramesCount = qFloor(duration*mFps/AV_TIME_BASE);
                } else {
                    mFramesCount = 0;
                    break;
                }
            }
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
    AVFormatContext *formatContext = avformat_alloc_context();
    if(!formatContext) {
        fprintf(stderr, "Error allocating AVFormatContext\n");
        return;// -1;
    }
    if(avformat_open_input(&formatContext, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return;// -1;
    }
    if(avformat_find_stream_info(formatContext, NULL) < 0) {
        fprintf(stderr,
                "Could not retrieve stream info from file '%s'\n",
                path);
        return;// -1;
    }

    // Find the index of the first audio stream
    int videoStreamIndex = -1;
    AVCodecParameters *codecPars = NULL;
    AVCodec *codec = NULL;
    AVStream *videoStream = NULL;
    for(uint i = 0; i < formatContext->nb_streams; i++) {
        AVStream *streamT = formatContext->streams[i];
        AVCodecParameters *codecParsT = streamT->codecpar;
        const AVMediaType &mediaType = codecParsT->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            codecPars = codecParsT;
            codec = avcodec_find_decoder(codecPars->codec_id);
            videoStream = formatContext->streams[videoStreamIndex];
            break;
        }
    }
    if(videoStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve video stream from file '%s'\n",
                path);
        return;// -1;
    }

    if(codec == NULL) {
        fprintf(stderr, "Unsuported codec\n");
        return;
    }
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if(!codecContext) {
        fprintf(stderr, "Error allocating AVCodecContext\n");
        return;// -1;
    }
    if(avcodec_parameters_to_context(codecContext, codecPars) < 0) {
        fprintf(stderr, "Failed to copy codec params to codec context\n");
        return;// -1;
    }

    if(avcodec_open2(codecContext, codec, NULL) < 0 ) {
        fprintf(stderr, "Failed to open codec\n");
        return;// -1;
    }
    struct SwsContext *sws = NULL;
    sws = sws_getContext(codecContext->width, codecContext->height,
                         codecContext->pix_fmt,
                         codecContext->width, codecContext->height,
                         AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);

    // prepare to read data
    AVPacket *packet = av_packet_alloc();
    if(!packet) {
        fprintf(stderr, "Error allocating AVPacket\n");
        return;// -1;
    }
    AVFrame *decodedFrame = av_frame_alloc();
    if(!decodedFrame) {
        fprintf(stderr, "Error allocating AVFrame\n");
        return;// -1;
    }

    QElapsedTimer framesTimer;
    framesTimer.start();
    int firstDts = 0;
    if(videoStream->first_dts != AV_NOPTS_VALUE) {
        firstDts = videoStream->first_dts;
    }
    bool frameReceived = false;
    foreach(const int &frameId, mFramesBeingLoaded) {
        int tsms = qRound(frameId * 1000 / mUpdateFps);

        int64_t frame = av_rescale(tsms, videoStream->time_base.den,
                                   videoStream->time_base.num)/1000;

        if(frameId != 0) {
            if(avformat_seek_file(formatContext, videoStreamIndex, 0,
                                  frame, frame,
                    AVSEEK_FLAG_FRAME) < 0) {
                return;// 0;
            }
        }

        avcodec_flush_buffers(codecContext);

        int64_t pts = 0;

        while(true) {
            if(av_read_frame(formatContext, packet) >= 0) {
                int response = 0;
                if(packet->stream_index == videoStreamIndex) {
                    response = avcodec_send_packet(codecContext, packet);
                    if(response < 0) {
                        fprintf(stderr, "Sending packet to the decoder failed\n");
                        return;
                    }
                    response = avcodec_receive_frame(codecContext, decodedFrame);
                    if(response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                        av_packet_unref(packet);
                        continue;
                    } else if(response < 0) {
                        fprintf(stderr, "Did not receive frame from the decoder\n");
                        return;
                    }
                    av_packet_unref(packet);
                } else {
                    av_packet_unref(packet);
                    continue;
                }
                //if(frameId == 0) break;
            } else {
                break;
            }

            // calculate PTS:
            pts = av_frame_get_best_effort_timestamp(decodedFrame);
            pts = av_rescale_q(pts, videoStream->time_base, AV_TIME_BASE_Q);
            if(pts/1000 > tsms) {
//                qDebug() << pts/1000 << tsms;
//                qDebug() << "for" << frameId << "received" << pts*mUpdateFps/1000000;
//                qDebug() << "seeked" << frame;
                frameReceived = true;
                break;
            }
            av_frame_unref(decodedFrame);
        }

    // SKIA

        if(frameReceived) {
            SkImageInfo info = SkImageInfo::Make(codecContext->width,
                                                 codecContext->height,
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
                      0, codecContext->height,
                      dstSk, linesizesSk);

            mLoadedFrames << SkImage::MakeFromBitmap(bitmap);
        } else {
            mLoadedFrames << sk_sp<SkImage>();
        }
        av_frame_unref(decodedFrame);
        av_packet_unref(packet);
    // SKIA
    }
//    qDebug() << "elapsed loading " <<
//                mFramesBeingLoaded.count() <<
//                " frames: " << framesTimer.elapsed();

    // clean up

    avformat_close_input(&formatContext);
    av_packet_free(&packet);
    sws_freeContext(sws);
    avcodec_close(codecContext);
    av_frame_free(&decodedFrame);
    avformat_free_context(formatContext);

//    qDebug() << "total elapsed: " << timer.elapsed();
    // success
    return;// 0;
}

void VideoCacheHandler::afterUpdate() {
    FileCacheHandler::afterUpdate();
//    qDebug() << "loaded: " << mFramesBeingLoaded;
    for(int i = 0; i < mFramesBeingLoaded.count(); i++) {
        int frameId = mFramesBeingLoaded.at(i);
        sk_sp<SkImage> imgT = mLoadedFrames.at(i);
        if(imgT.get() == NULL) {
            mFramesCount = frameId;
            foreach(const BoundingBoxQWPtr &boxWPtr, mDependentBoxes) {
                BoundingBox *box = boxWPtr.data();
                if(box == NULL) continue;
                ((VideoBox*)box)->updateDurationRectangleAnimationRange();
            }
        } else {
            CacheContainer *cont =
                    mFramesCache.createNewRenderContainerAtRelFrame(frameId);
            cont->replaceImageSk(imgT);
        }
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
    if(mFramesCount <= 0 || frame >= mFramesCount) return NULL;
    if(mFramesLoadScheduled.contains(frame) ||
       mFramesBeingLoadedGUI.contains(frame)) return this;
//    qDebug() << "schedule frame load: " << frame;
    CacheContainer *contAtFrame =
            mFramesCache.getRenderContainerAtRelFrame(frame);
    if(contAtFrame == NULL) {
        mFramesLoadScheduled << frame;
    } else {
        return contAtFrame->scheduleLoadFromTmpFile();
    }
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
           extension == "mov"||
           extension == "mkv";
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
