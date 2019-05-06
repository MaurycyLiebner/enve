#ifndef AUDIOSTREAMSDATA_H
#define AUDIOSTREAMSDATA_H
#include "videoframeloader.h"

struct AudioStreamsData {
    QString fPath;
    bool fOpened = false;
    qreal fFps = 0;
    int fDurationSec = 0;
    int fTimeBaseNum = 0;
    int fTimeBaseDen = 1;
    AVFormatContext *fFormatContext = nullptr;
    int fAudioStreamIndex = -1;
    AVStream * fAudioStream = nullptr;
    AVPacket * fPacket = nullptr;
    AVFrame *fDecodedFrame = nullptr;
    AVCodecContext * fCodecContext = nullptr;
    struct SwrContext * fSwrContext = nullptr;

    void open(const QString& path);
    void close();
private:
    void open();
    void open(const char * const path);
};

#endif // AUDIOSTREAMSDATA_H
