//#ifndef SOUNDCOMPOSITION_H
//#define SOUNDCOMPOSITION_H

//#include <QIODevice>
//#include <QAudioBuffer>

//class SingleSound;

//class SoundComposition : public QIODevice
//{
//    Q_OBJECT
//public:
//    SoundComposition(QObject *parent = 0);
//    ~SoundComposition();

//    void setFirstAndLastVideoFrame(const int &firstVideoFrame,
//                                   const int &lastVideoFrame);
//    void generateData();

//    void start();
//    void stop();
//    qint64 readData(char *data, qint64 len);
//    qint64 writeData(const char *data, qint64 len);
//    qint64 bytesAvailable() const;

//    void addSound(SingleSound *sound);
//    void removeSound(SingleSound *sound);
//private:
//    QList<SingleSound*> mSounds;
//    QAudioBuffer::S16U *mData = NULL;
//    QByteArray mBuffer = NULL;

//    qint64 mFirstAudioFrame;
//    qint64 mLastAudioFrame;

//    qint64 mPos;
//signals:

//};

//#endif // SOUNDCOMPOSITION_H
