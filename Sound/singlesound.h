//#ifndef SINGLESOUND_H
//#define SINGLESOUND_H
//#include <QString>
//#include <QObject>
//#include <QAudioFormat>
//#include <QAudioDecoder>
//#include "qrealanimator.h"
//#include <QVector>

//class SingleSound : public QObject
//{
//    Q_OBJECT
//public:
//    SingleSound(const QString &fileName);

//    const unsigned char *getConstData() const;
//    void reload();

//    int getStartAudioFrame();
//    int getAudioFrameLength();

//    qint64 getCurrentBufferStartAudioFrame();
//    qint64 getCurrentBufferLength();
//public slots:
//    void processBuffer();
//    void error();
//    void processAfterFinished();
//private:
//    QAudioFormat mSrcFormat;
//    int mStartVideoFrame = 0;
//    QString mFileName;
//    QAudioDecoder *mDecoder;
//    QAudioBuffer mBuffer;
//    QVector<unsigned char> mData;
//    //QrealAnimator mVolumeAnimator;
//    //qreal mScale = 100.;
//};

//#endif // SINGLESOUND_H
