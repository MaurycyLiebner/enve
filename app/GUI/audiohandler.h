#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H
#include <QAudioOutput>

class AudioHandler : public QObject {
public:
    AudioHandler();

    struct DataRequest {
        char* fData;
        int fSize;

        operator bool() const {
            return fSize > 0;
        }
    };

    DataRequest dataRequest() {
        if(mAudioOutput && mAudioOutput->state() != QAudio::StoppedState) {
            if(mAudioOutput->bytesFree() <= 0) return {nullptr, 0};
            const int size = qMin(mAudioOutput->periodSize(),
                                  mAudioOutput->bytesFree());
            return {mAudioBuffer.data(), size};
        }
        return {nullptr, 0};
    }

    void provideData(const DataRequest& request) {
        if(!request) return;
        mAudioIOOutput->write(request.fData, request.fSize);
    }

    void initializeAudio();
    void startAudio();
    void stopAudio();
    void setVolume(const int value);
private:
    QAudioDeviceInfo mAudioDevice;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioIOOutput; // not owned
    QAudioFormat mAudioFormat;

    QByteArray mAudioBuffer;
};

#endif // AUDIOHANDLER_H
