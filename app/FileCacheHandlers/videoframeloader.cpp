#include "videoframeloader.h"
#include "videocachehandler.h"

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

    av_frame_unref(mFrameToConvert);
    sws_freeContext(mSwsContext);
    mFrameToConvert = nullptr;
    mSwsContext = nullptr;
}

int frameId(AVFrame * const decodedFrame,
            AVStream * const videoStream,
            const qreal& fps) {
    int64_t pts = av_frame_get_best_effort_timestamp(decodedFrame);
    pts = av_rescale_q(pts, videoStream->time_base, AV_TIME_BASE_Q);
    return qRound(pts/1000000.*fps);
}

void VideoFrameLoader::readFrame() {
    if(!mOpenedVideo->fOpened)
        RuntimeThrow("Cannot read frame from closed VideoStream");
    const auto formatContext = mOpenedVideo->fFormatContext;
    const auto videoStreamIndex = mOpenedVideo->fVideoStreamIndex;
    const auto videoStream = mOpenedVideo->fVideoStream;
    const auto packet = mOpenedVideo->fPacket;
    const auto decodedFrame = mOpenedVideo->fDecodedFrame;
    const auto codecContext = mOpenedVideo->fCodecContext;
    //const auto swsContext = mOpenedVideo->fSwsContext;
    const qreal fps = mOpenedVideo->fFps;

    if(mOpenedVideo->fLastFrame >= mFrameId ||
       mFrameId - mOpenedVideo->fLastFrame > fps) {
        const int64_t tsms = qMax(0, qFloor((mFrameId - 1) * 1000 / fps));
        const int64_t tm = av_rescale(tsms, videoStream->time_base.den,
                                      videoStream->time_base.num)/1000;
        if(tm <= 0)
            avformat_seek_file(formatContext, videoStreamIndex,
                               INT64_MIN, 0, 0, 0);
        else {
            const int64_t tsms0 = qMax(0, qFloor((mFrameId - fps) * 1000 / fps));
            const int64_t tm0 = av_rescale(tsms0, videoStream->time_base.den,
                                           videoStream->time_base.num)/1000;
            if(avformat_seek_file(formatContext, videoStreamIndex, tm0,
                                  tm, tm, AVSEEK_FLAG_FRAME) < 0) {
                qDebug() << "Failed to seek to " << mFrameId;
                avformat_seek_file(formatContext, videoStreamIndex,
                                   INT64_MIN, 0, INT64_MAX, 0);
            }
        }
        avcodec_flush_buffers(codecContext);
    }

    bool frameReceived = false;
    while(true) {
        if(av_read_frame(formatContext, packet) < 0) {
            break;
        } else {
            if(packet->stream_index == videoStreamIndex) {
                const int sendRet = avcodec_send_packet(codecContext, packet);
                if(sendRet < 0) {
                    RuntimeThrow("Sending packet to the decoder failed");
                }
                const int recRet = avcodec_receive_frame(codecContext, decodedFrame);
                if(recRet == AVERROR_EOF) {
                    return;
                } else if(recRet == AVERROR(EAGAIN)) {
                    av_packet_unref(packet);
                    continue;
                } else if(recRet < 0) {
                    RuntimeThrow("Did not receive frame from the decoder");
                }
                av_packet_unref(packet);
            } else {
                av_packet_unref(packet);
                continue;
            }
        }

        const int currFrame = frameId(decodedFrame, videoStream, fps);
        mOpenedVideo->fLastFrame = currFrame;
        qDebug() << "got " << currFrame << " for " << mFrameId;
        if(currFrame >= mFrameId) {
            if(currFrame > mFrameId)
                qDebug() << "video" << QString::number(currFrame) +
                            " instead of " + QString::number(mFrameId);
            frameReceived = true;
            break;
        }
        av_frame_unref(decodedFrame);
    }

    av_packet_unref(packet);
    if(frameReceived) {
        installFrame(decodedFrame, codecContext);
        HDDPartFinished();
        convertFrame();
    } else {
        mLoadedFrame.reset();
        av_frame_unref(decodedFrame);
    }
}

void VideoFrameLoader::afterProcessing() {
    mCacheHandler->frameLoaderFinished(mFrameId, mLoadedFrame);
}

void VideoFrameLoader::afterCanceled() {
    mCacheHandler->frameLoaderCanceled(mFrameId);
}
