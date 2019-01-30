#include "videocachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "Boxes/videobox.h"
extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <QFileDialog>
#include "GUI/mainwindow.h"
#include "filesourcescache.h"

VideoCacheHandler::VideoCacheHandler(const QString &filePath) :
    AnimationCacheHandler(filePath) {
    updateFrameCount();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtFrame(const int &relFrame) {
    ImageCacheContainer *cont = mFramesCache.getRenderContainerAtRelFrame(relFrame);
    if(cont == nullptr) return sk_sp<SkImage>();
    //cont->neededInMemory();
    return cont->getImageSk();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtOrBeforeFrame(const int& relFrame) {
    ImageCacheContainer *cont =
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

void VideoCacheHandler::_processUpdate() {
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
            if(av_read_frame(formatContext, packet) < 0) {
                break;
            } else {
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
        if(imgT) {
            mFramesCache.createNewRenderContainerAtRelFrame(frameId, imgT);
        } else {
            mFramesCount = frameId;
            foreach(const auto &box, mDependentBoxes) {
                if(!box) continue;
                GetAsPtr(box, VideoBox)->updateDurationRectangleAnimationRange();
            }
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
    ImageCacheContainer *contAtFrame =
            mFramesCache.getRenderContainerAtRelFrame(frame);
    if(contAtFrame) {
        return contAtFrame->scheduleLoadFromTmpFile();
    } else {
        mFramesLoadScheduled << frame;
    }
    scheduleTask();
    return this;
}
