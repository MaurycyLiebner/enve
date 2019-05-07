#ifndef AUDIOSTREAMSDATA_H
#define AUDIOSTREAMSDATA_H
#include "soundreader.h"

struct AudioStreamsData {
private:
    explicit AudioStreamsData() {}

    ~AudioStreamsData() {
        if(fOpened) close();
    }

    static void sDestroy(AudioStreamsData * const p) {
        delete p;
    }
protected:
    friend struct VideoStreamsData;
    static stdsptr<const AudioStreamsData> sOpen(
            const QString& path,
            AVFormatContext * const formatContext);
public:
    QString fPath;
    bool fOpened = false;
    int fDurationSec = 0;
    AVFormatContext *fFormatContext = nullptr;
    int fAudioStreamIndex = -1;
    AVStream * fAudioStream = nullptr;
    AVPacket * fPacket = nullptr;
    AVFrame *fDecodedFrame = nullptr;
    AVCodecContext * fCodecContext = nullptr;
    struct SwrContext * fSwrContext = nullptr;

    static stdsptr<const AudioStreamsData> sOpen(const QString& path);
private:
    void open(const QString& path, AVFormatContext * const formatContext);
    void open(const QString& path);
    void open();
    void open(const char * const path);
    void open(AVFormatContext * const formatContext);

    void close();
};

#endif // AUDIOSTREAMSDATA_H
