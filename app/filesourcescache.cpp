#include "filesourcescache.h"
#include "Boxes/rendercachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "GUI/mainwindow.h"
#include "Boxes/boundingbox.h"
#include "GUI/filesourcelist.h"
#include "Boxes/videobox.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <QFileDialog>

void FileSourcesCache::addFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    mFileSourceListVisibleWidgets << wid;
}

void FileSourcesCache::removeFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    mFileSourceListVisibleWidgets.removeOne(wid);
}

void FileSourcesCache::addHandlerToHandlersList(
        const stdsptr<FileCacheHandler> &handlerPtr) {
    mFileCacheHandlers.append(handlerPtr);
}

void FileSourcesCache::addHandlerToListWidgets(FileCacheHandler *handlerPtr) {
    foreach(FileSourceListVisibleWidget *wid, mFileSourceListVisibleWidgets) {
        wid->addCacheHandlerToList(handlerPtr);
    }
}

FileCacheHandler *FileSourcesCache::getHandlerForFilePath(
        const QString &filePath) {
    Q_FOREACH(const stdsptr<FileCacheHandler> &handler,
              mFileCacheHandlers) {
        if(handler->getFilePath() == filePath) {
            return handler.get();
        }
    }
    QString ext = filePath.split(".").last();
    if(isVideoExt(ext)) {
        return VideoCacheHandler::createNewHandler(filePath);
    } else if(isImageExt(ext)) {
        return ImageCacheHandler::createNewHandler(filePath);
    }
    return nullptr;
}

void FileSourcesCache::removeHandler(const stdsptr<FileCacheHandler>& handler) {
    mFileCacheHandlers.removeOne(handler);
    foreach(FileSourceListVisibleWidget *wid, mFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handler.get());
    }
}

void FileSourcesCache::removeHandlerFromListWidgets(FileCacheHandler *handlerPtr) {
    foreach(FileSourceListVisibleWidget *wid, mFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handlerPtr);
    }
}

void FileSourcesCache::clearAll() {
    Q_FOREACH(const stdsptr<FileCacheHandler> &handler,
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
    QFile file(mFilePath);
    mFileMissing = !file.exists();
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

void FileCacheHandler::clearCache() {
    QFile file(mFilePath);
    mFileMissing = !file.exists();
    foreach(const qptr<BoundingBox> &boxPtr, mDependentBoxes) {
        BoundingBox *box = boxPtr.data();
        if(box == nullptr) continue;
        box->reloadCacheHandler();
    }
}

void FileCacheHandler::addDependentBox(BoundingBox *dependent) {
    mDependentBoxes << dependent;
}

void FileCacheHandler::removeDependentBox(BoundingBox *dependent) {
    for(int i = 0; i < mDependentBoxes.count(); i++) {
        const qptr<BoundingBox> &boxPtr = mDependentBoxes.at(i);
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

void ImageCacheHandler::_processUpdate() {
    sk_sp<SkData> data = SkData::MakeFromFileName(
                mFilePath.toLocal8Bit().data());
    mUpdateImage = SkImage::MakeFromEncoded(data);
}

void ImageCacheHandler::afterProcessingFinished() {
    FileCacheHandler::afterProcessingFinished();
    mImage = mUpdateImage;
    mUpdateImage.reset();
}

VideoCacheHandler::VideoCacheHandler(const QString &filePath) :
    AnimationCacheHandler(filePath) {
    updateFrameCount();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtFrame(const int &relFrame) {
    CacheContainer *cont = mFramesCache.getRenderContainerAtRelFrame(relFrame);
    if(cont == nullptr) return sk_sp<SkImage>();
    //cont->neededInMemory();
    return cont->getImageSk();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtOrBeforeFrame(const int& relFrame) {
    CacheContainer *cont =
            mFramesCache.getRenderContainerAtOrBeforeRelFrame(
                relFrame);
    if(cont == nullptr) return sk_sp<SkImage>();
    //cont->neededInMemory();
    return cont->getImageSk();
}

void VideoCacheHandler::beforeProcessingStarted() {
    FileCacheHandler::beforeProcessingStarted();
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
    if(avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return;
    }
    if(avformat_find_stream_info(format, nullptr) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return;
    }

    // Find the index of the first audio stream
    for(uint i = 0; i < format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            AVStream *vidStream = streamT;
            mTimeBaseDen = vidStream->avg_frame_rate.den;
            mTimeBaseNum = vidStream->avg_frame_rate.num;
            if(mTimeBaseDen != 0) {
                mFps = static_cast<qreal>(mTimeBaseNum/mTimeBaseDen);
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
                        mFps = mTimeBaseNum/static_cast<qreal>(mTimeBaseDen);
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
void VideoCacheHandler::_processUpdate() {
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
    if(avformat_open_input(&formatContext, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return;// -1;
    }
    if(avformat_find_stream_info(formatContext, nullptr) < 0) {
        fprintf(stderr,
                "Could not retrieve stream info from file '%s'\n",
                path);
        return;// -1;
    }

    // Find the index of the first audio stream
    int videoStreamIndex = -1;
    AVCodecParameters *codecPars = nullptr;
    AVCodec *codec = nullptr;
    AVStream *videoStream = nullptr;
    for(uint i = 0; i < formatContext->nb_streams; i++) {
        AVStream *streamT = formatContext->streams[i];
        AVCodecParameters *codecParsT = streamT->codecpar;
        const AVMediaType &mediaType = codecParsT->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = static_cast<int>(i);
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

    if(codec == nullptr) {
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

    if(avcodec_open2(codecContext, codec, nullptr) < 0 ) {
        fprintf(stderr, "Failed to open codec\n");
        return;// -1;
    }
    struct SwsContext *sws = nullptr;
    sws = sws_getContext(codecContext->width, codecContext->height,
                         codecContext->pix_fmt,
                         codecContext->width, codecContext->height,
                         AV_PIX_FMT_BGRA, SWS_BICUBIC, nullptr, nullptr, nullptr);

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

void VideoCacheHandler::afterProcessingFinished() {
    FileCacheHandler::afterProcessingFinished();
//    qDebug() << "loaded: " << mFramesBeingLoaded;
    for(int i = 0; i < mFramesBeingLoaded.count() &&
        i < mLoadedFrames.count(); i++) {
        int frameId = mFramesBeingLoaded.at(i);
        sk_sp<SkImage> imgT = mLoadedFrames.at(i);
        if(imgT.get() == nullptr) {
            mFramesCount = frameId;
            foreach(const qptr<BoundingBox> &box, mDependentBoxes) {
                if(box == nullptr) continue;
                GetAsPtr(box, VideoBox)->updateDurationRectangleAnimationRange();
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

void VideoCacheHandler::replace() {
    QString importPath = QFileDialog::getOpenFileName(
                MainWindow::getInstance(),
                "Replace Video Source " + mFilePath, "",
                "Files (*.mp4 *.mov *.avi *.mkv *.m4v)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPath.isEmpty()) {
        QFile file(importPath);
        if(!file.exists()) return;
        if(hasVideoExt(importPath)) {
            mFilePath = importPath;
            updateFrameCount();
            clearCache();
        }
    }
}

const qreal &VideoCacheHandler::getFps() { return mFps; }

_ScheduledTask* VideoCacheHandler::scheduleFrameLoad(
        const int &frame) {
    if(mFramesCount <= 0 || frame >= mFramesCount) return nullptr;
    if(mFramesLoadScheduled.contains(frame) ||
            mFramesBeingLoadedGUI.contains(frame)) return this;
    //    qDebug() << "schedule frame load: " << frame;
    CacheContainer *contAtFrame =
            mFramesCache.getRenderContainerAtRelFrame(frame);
    if(contAtFrame == nullptr) {
        mFramesLoadScheduled << frame;
    } else {
        return contAtFrame->scheduleLoadFromTmpFile();
    }
    scheduleTask();
    return this;
}

ImageSequenceCacheHandler::ImageSequenceCacheHandler(
        const QStringList &framePaths) {
    mFramePaths = framePaths;
    foreach(const QString &path, framePaths) {
        auto imgCacheHandler = GetAsPtr(
                FileSourcesCache::getHandlerForFilePath(path),
                    ImageCacheHandler);
        if(imgCacheHandler == nullptr) {
            auto newHandler = ImageCacheHandler::createNewHandler(path, false);
            mFrameImageHandlers << newHandler;
        } else {
            mFrameImageHandlers << GetAsPtr(imgCacheHandler, ImageCacheHandler);
        }
    }
    updateFrameCount();
}

sk_sp<SkImage> ImageSequenceCacheHandler::getFrameAtFrame(const int &relFrame) {
    ImageCacheHandler *cacheHandler = mFrameImageHandlers.at(relFrame);
    if(cacheHandler == nullptr) return sk_sp<SkImage>();
    return cacheHandler->getImage();
}

sk_sp<SkImage> ImageSequenceCacheHandler::getFrameAtOrBeforeFrame(
        const int &relFrame) {
    if(mFrameImageHandlers.isEmpty()) return sk_sp<SkImage>();
    if(relFrame >= mFrameImageHandlers.count()) {
        return mFrameImageHandlers.last()->getImage();
    }
    ImageCacheHandler *cacheHandler = mFrameImageHandlers.at(relFrame);
    return cacheHandler->getImage();
}

void ImageSequenceCacheHandler::updateFrameCount() {
    mFramesCount = mFramePaths.count();
}

void ImageSequenceCacheHandler::clearCache() {
    foreach(const stdptr<ImageCacheHandler> &cacheHandler, mFrameImageHandlers) {
        cacheHandler->clearCache();
    }
    FileCacheHandler::clearCache();
}

_ScheduledTask *ImageSequenceCacheHandler::scheduleFrameLoad(
        const int &frame) {
    ImageCacheHandler* imageHandler = mFrameImageHandlers.at(frame);
    return imageHandler;
}

bool isVideoExt(const QString &extension) {
    return extension == "avi" ||
           extension == "mp4" ||
           extension == "mov" ||
           extension == "mkv" ||
           extension == "m4v";
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
extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}
#include "Sound/soundcomposition.h"
int SoundCacheHandler::decodeSoundDataRange(SoundDataRange &range) {
    float **audioData = &range.data;
    QByteArray pathBytes = mUpdateFilePath.toLatin1();
    const char* path = pathBytes.data();
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if(avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if(avformat_find_stream_info(format, nullptr) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int audioStreamIndex = -1;
    for(uint i = 0; i< format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }
    if(audioStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVCodecContext *audioCodec = nullptr;
    struct SwrContext *swr = nullptr;

    AVStream* audioStream = format->streams[audioStreamIndex];
    // find & open codec
    audioCodec = audioStream->codec;
    if(avcodec_open2(audioCodec, avcodec_find_decoder(audioCodec->codec_id), nullptr) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n",
                audioStreamIndex, path);
        return -1;
    }

    // prepare resampler
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  audioCodec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 1, 0);
    av_opt_set_int(swr, "in_channel_layout",  audioCodec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "in_sample_rate", audioCodec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", SOUND_SAMPLERATE, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  audioCodec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
    swr_init(swr);
    if(!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* audioFrame = av_frame_alloc();
    if(!audioFrame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    *audioData = nullptr;
    int nSamples = 0;
    int maxFrameTsms = qRound(range.maxRelFrame * 1000 / mUpdateFps);
    int minFrameTsms = qRound(range.minRelFrame * 1000 / mUpdateFps);

    int64_t seekFrame = av_rescale(minFrameTsms,
                                   audioStream->time_base.den,
                                   audioStream->time_base.num)/1000;

    if(range.minRelFrame != 0) {
        if(avformat_seek_file(format, audioStreamIndex, 0,
                              seekFrame, seekFrame,
                AVSEEK_FLAG_FRAME) < 0) {
            fprintf(stderr, "Error seeking the audio frame\n");
            return -1;// 0;
        }
    }
    bool firstFrame = true;
    while(av_read_frame(format, &packet) >= 0) {
        if(packet.stream_index == audioStreamIndex) {
            // decode one frame
            int gotFrame;
            if(avcodec_decode_audio4(audioCodec, audioFrame, &gotFrame, &packet) < 0) {
                break;
            }
            if(!gotFrame) {
                continue;
            }
            // resample frames
            float *buffer;
            av_samples_alloc((uint8_t**) &buffer, nullptr, 1,
                             audioFrame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
            int nSamplesT = swr_convert(swr,
                                          (uint8_t**) &buffer,
                                          audioFrame->nb_samples,
                                          (const uint8_t**) audioFrame->data,
                                          audioFrame->nb_samples);
            // append resampled frames to data
            *audioData = (float*) realloc(*audioData,
                                     (nSamples + audioFrame->nb_samples) * sizeof(float));
            memcpy(*audioData + nSamples, buffer, nSamplesT * sizeof(float));

            av_freep(&((uint8_t**) &buffer)[0]);
            nSamples += nSamplesT;

            if(firstFrame) {
                firstFrame = false;
                int64_t pts = av_frame_get_best_effort_timestamp(audioFrame);
                pts = av_rescale_q(pts, audioStream->time_base, AV_TIME_BASE_Q);
                range.minSample = pts*SOUND_SAMPLERATE/1000000;
            }
            int64_t pts = av_frame_get_best_effort_timestamp(audioFrame);
            pts = av_rescale_q(pts, audioStream->time_base, AV_TIME_BASE_Q);
            if(pts/1000 > maxFrameTsms) {
                range.maxSample = range.minSample + nSamples;
                range.updateSampleCount();
                break;
            }
        }

        av_free_packet(&packet);
    }

    // clean up
    av_frame_free(&audioFrame);
    if(swr != nullptr) {
        swr_free(&swr);
    }
    if(audioCodec != nullptr) {
        avcodec_close(audioCodec);
    }
    avformat_free_context(format);

    // success
    return 0;
}

void SoundCacheHandler::_processUpdate() {
    for(int i = 0; i < mSoundBeingLoaded.count(); i++) {
        SoundDataRange range = mSoundBeingLoaded.at(i);

        decodeSoundDataRange(range);

        mSoundBeingLoaded.replace(i, range);
    }
}

bool hasVideoExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isVideoExt(extension);
}

bool hasSoundExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isSoundExt(extension);
}

bool hasVectorExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isVectorExt(extension);
}

bool hasImageExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isImageExt(extension);
}

bool hasAvExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isAvExt(extension);
}
