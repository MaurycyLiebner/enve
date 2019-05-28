#include "videoframeloader.h"
#include "videocachehandler.h"
#include "taskscheduler.h"

VideoFrameLoader::VideoFrameLoader(VideoCacheHandler * const cacheHandler,
                                   const stdsptr<VideoStreamsData> &openedVideo,
                                   const int &frameId, AVFrame * const frame) :
    VideoFrameLoader(cacheHandler, openedVideo, frameId) {
    setFrameToConvert(frame, openedVideo->fCodecContext);
}

void VideoFrameLoader::convertFrame() {
    const auto info = SkiaHelpers::getPremulBGRAInfo(
                mFrameToConvert->width, mFrameToConvert->height);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    SkPixmap pixmap;
    bitmap.peekPixels(&pixmap);

    void * const addr = pixmap.writable_addr();
    uint8_t * const dstSk[] = { static_cast<uint8_t*>(addr) };
    int linesizesSk[4];

    av_image_fill_linesizes(linesizesSk, AV_PIX_FMT_BGRA,
                            mFrameToConvert->width);

    sws_scale(mSwsContext, mFrameToConvert->data, mFrameToConvert->linesize,
              0, mFrameToConvert->height, dstSk, linesizesSk);

    mLoadedFrame = SkiaHelpers::transferDataToSkImage(bitmap);

    cleanUp();
}

int frameId(AVFrame * const decodedFrame,
            AVStream * const videoStream,
            const qreal& fps) {
    int64_t pts = av_frame_get_best_effort_timestamp(decodedFrame);
    pts = av_rescale_q(pts, videoStream->time_base, AV_TIME_BASE_Q);
    return qRound(pts/1000000.*fps);
}

void seek(const int& tryN, const int& frameId, const qreal& fps,
          AVFormatContext * const formatContext,
          const int& videoStreamIndex, AVStream * const videoStream,
          AVCodecContext * const codecContext) {
    const int64_t tsms = qMax(0, qFloor((frameId - tryN) * 1000 / fps));
    const int64_t tm = av_rescale(tsms, videoStream->time_base.den,
                                  videoStream->time_base.num)/1000;
    if(tm <= 0)
        avformat_seek_file(formatContext, videoStreamIndex,
                           INT64_MIN, 0, 0, 0);
    else {
        const int64_t tsms0 = qMax(0, qFloor((frameId - fps - tryN) * 1000 / fps));
        const int64_t tm0 = av_rescale(tsms0, videoStream->time_base.den,
                                       videoStream->time_base.num)/1000;
        if(avformat_seek_file(formatContext, videoStreamIndex, tm0,
                              tm, tm, AVSEEK_FLAG_FRAME) < 0) {
            qDebug() << "Failed to seek to " << frameId;
            avformat_seek_file(formatContext, videoStreamIndex,
                               INT64_MIN, 0, INT64_MAX, 0);
        }
    }
    avcodec_flush_buffers(codecContext);
}

void VideoFrameLoader::readFrame() {
    if(!mOpenedVideo->fOpened)
        RuntimeThrow("Cannot read frame from closed VideoStream");
    const auto formatContext = mOpenedVideo->fFormatContext;
    const auto videoStreamIndex = mOpenedVideo->fVideoStreamIndex;
    const auto videoStream = mOpenedVideo->fVideoStream;
    const auto packet = mOpenedVideo->fPacket;
    const auto codecContext = mOpenedVideo->fCodecContext;
    auto decodedFrame = mOpenedVideo->fDecodedFrame;

    //const auto swsContext = mOpenedVideo->fSwsContext;
    const qreal fps = mOpenedVideo->fFps;

    int seekTry = 0;
    if(mOpenedVideo->fLastFrame >= mFrameId ||
       mFrameId - mOpenedVideo->fLastFrame > fps) {
        seek(seekTry++, mFrameId, fps, formatContext,
             videoStreamIndex, videoStream, codecContext);
    }

    while(true) {
        mOpenedVideo->fLastFrame = -qFloor(10*fps); // Just in case error occurs
        if(av_read_frame(formatContext, packet) < 0) {
            //RuntimeThrow("Error retrieving AVPacket");
            break;
        } else {
            if(packet->stream_index == videoStreamIndex) {
                const int sendRet = avcodec_send_packet(codecContext, packet);
                if(sendRet < 0) RuntimeThrow("Sending packet to the decoder failed");

                const int recRet = avcodec_receive_frame(codecContext, decodedFrame);
                av_packet_unref(packet);

                if(recRet == AVERROR_EOF)
                    break;
                else if(recRet == AVERROR(EAGAIN)) {
                    continue;
                } else if(recRet < 0)
                    RuntimeThrow("Did not receive frame from the decoder");
            } else {
                av_packet_unref(packet);
                continue;
            }
        }

        const int currFrame = frameId(decodedFrame, videoStream, fps);
        mOpenedVideo->fLastFrame = currFrame;
        const bool reseek = currFrame > mFrameId && seekTry <= 3;
        if(currFrame == mFrameId || (!reseek && currFrame > mFrameId)) {
            if(currFrame > mFrameId)
                qDebug() << "frame" << QString::number(currFrame) +
                            " instead of " + QString::number(mFrameId);            
            setFrameToConvert(decodedFrame, codecContext);
            mOpenedVideo->fDecodedFrame = av_frame_alloc();
            break;
        } else if(qAbs(mFrameId - currFrame) < 20) {
            mExcessFrames.append({currFrame, decodedFrame});
            mOpenedVideo->fDecodedFrame = av_frame_alloc();
            decodedFrame = mOpenedVideo->fDecodedFrame;
        } else av_frame_unref(decodedFrame);

        if(reseek) seek(seekTry++, mFrameId, fps, formatContext,
                        videoStreamIndex, videoStream, codecContext);
    }
}

void VideoFrameLoader::afterProcessing() {
    mCacheHandler->frameLoaderFinished(mFrameId, mLoadedFrame);
    for(auto& excess : mExcessFrames) {
        if(mCacheHandler->getFrameAtFrame(excess.first)) {
            av_frame_unref(excess.second);
            av_frame_free(&excess.second);
            continue;
        }
        const auto currFL = mCacheHandler->getFrameLoader(excess.first);
        if(currFL) {
            currFL->setFrameToConvert(excess.second, mOpenedVideo->fCodecContext);
        } else {
            const auto newFL = mCacheHandler->addFrameLoader(excess.first, excess.second);
            newFL->scheduleTask();
        }
    }
    mExcessFrames.clear();
}

void VideoFrameLoader::afterCanceled() {
    mCacheHandler->frameLoaderCanceled(mFrameId);
}

void VideoFrameLoader::scheduleTaskNow() {
    if(mFrameToConvert) {
        TaskScheduler::sGetInstance()->scheduleCPUTask(ref<Task>());
    } else {
        TaskScheduler::sGetInstance()->scheduleHDDTask(ref<Task>());
    }
}

void VideoFrameLoader::processTask() {
    if(!mFrameToConvert) readFrame();
    if(mFrameToConvert) {
        HDDPartFinished();
        convertFrame();
    }
}
