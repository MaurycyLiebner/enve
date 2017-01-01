//#include "singlesound.h"

//SingleSound::SingleSound(const QString &fileName) :
//    QObject() {
//    mFileName = fileName;
//    mDecoder = new QAudioDecoder(this);

//    connect(mDecoder, SIGNAL(bufferReady()), this, SLOT(processBuffer()));
//    connect(mDecoder, SIGNAL(error(QAudioDecoder::Error)),
//            this, SLOT(error()));
//    connect(mDecoder, SIGNAL(finished()),
//            this, SLOT(processAfterFinished()));
//    reload();
//}

//const unsigned char *SingleSound::getConstData() const
//{
//    return mData;
//}

//void SingleSound::reload() {
//    mBuffer = QAudioBuffer();
////    QAudioFormat desiredFormat;
////    desiredFormat.setChannelCount(2);
////    desiredFormat.setCodec("audio/x-raw");
////    desiredFormat.setSampleType(QAudioFormat::UnSignedInt);
////    desiredFormat.setSampleRate(48000);
////    desiredFormat.setSampleSize(16);
////    QAudioFormat format;
////    format.setChannelCount(2);
////    format.setSampleSize(16);
////    format.setSampleRate(48000);
////    format.setCodec("audio/pcm");
////    format.setSampleType(QAudioFormat::SignedInt);

//    //mDecoder->setAudioFormat(format);
//    mDecoder->setSourceFilename(mFileName);

//    mData.clear();
//    mDecoder->start();
//}

//int SingleSound::getStartAudioFrame() {
//    return mStartVideoFrame*48000;
//}

//int SingleSound::getAudioFrameLength() {
//    return mBuffer.frameCount();
//}

//#include <QDebug>
//void SingleSound::error() {
//    qDebug() << mDecoder->errorString() << mDecoder->error();
//}

//void SingleSound::processAfterFinished() {
//    mSrcFormat = mBuffer.format();


//}

//void SingleSound::processBuffer()
//{
//    mBuffer = mDecoder->read();

//    unsigned char *data = mBuffer.constData<unsigned char>();
//    qint64 nFrames = mBuffer.frameCount();

//    for(qint64 i = 0; i < nFrames; i++) {
//        mData.append(data[i]);
//    }
//}
