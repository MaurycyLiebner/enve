#ifndef AUDIOSTREAMSDATA_H
#define AUDIOSTREAMSDATA_H
#include "videoframeloader.h"

struct AudioStreamsData {
private:
    explicit AudioStreamsData() {}
    AudioStreamsData(const AudioStreamsData &) = delete;
    const AudioStreamsData &operator =(const AudioStreamsData &) = delete;
    ~AudioStreamsData() {
        if(fOpened) close();
    }

    static void sDestroy(AudioStreamsData * const p) {
        delete p;
    }
public:
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

    static stdsptr<const AudioStreamsData> sOpen(const QString& path);
private:
    void open(const QString& path);
    void open(const char * const path);
    void open();
    void close();
};

#endif // AUDIOSTREAMSDATA_H
