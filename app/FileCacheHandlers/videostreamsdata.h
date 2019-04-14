#ifndef VIDEOSTREAMSDATA_H
#define VIDEOSTREAMSDATA_H
#include "videoframeloader.h"

struct VideoStreamsData {
    QString fPath;
    bool fOpened = false;
    qreal fFps = 0;
    int fTimeBaseNum = 0;
    int fTimeBaseDen = 1;
    int fFrameCount = 0;
    AVFormatContext *fFormatContext = nullptr;
    int fVideoStreamIndex = -1;
    AVStream * fVideoStream = nullptr;
    AVPacket * fPacket = nullptr;
    AVFrame *fDecodedFrame = nullptr;
    AVCodecContext * fCodecContext = nullptr;
    struct SwsContext * fSwsContext = nullptr;

    void open(const QString& path);
    void close();
private:
    void open();
    void open(const char * const path);
};
#endif // VIDEOSTREAMSDATA_H
