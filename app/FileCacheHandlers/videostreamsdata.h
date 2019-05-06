#ifndef VIDEOSTREAMSDATA_H
#define VIDEOSTREAMSDATA_H
#include "videoframeloader.h"

struct VideoStreamsData {
private:
    explicit VideoStreamsData() {}

    ~VideoStreamsData() {
        if(fOpened) close();
    }

    static void sDestroy(VideoStreamsData * const p) {
        delete p;
    }
public:
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

    static stdsptr<const VideoStreamsData> sOpen(const QString& path);
private:
    void open(const QString& path);
    void open(const char * const path);
    void open();
    void close();
};
#endif // VIDEOSTREAMSDATA_H
