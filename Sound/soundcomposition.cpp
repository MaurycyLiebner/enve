//#include "soundcomposition.h"
//#include "singlesound.h"
//#include <qendian.h>

//SoundComposition::SoundComposition(QObject *parent) :
//    QIODevice(parent) {
//    mPos = 0;
//}

//void SoundComposition::setFirstAndLastVideoFrame(const int &firstVideoFrame,
//                                                 const int &lastVideoFrame) {
//    mFirstAudioFrame = firstVideoFrame*48000/24;
//    mLastAudioFrame = lastVideoFrame*48000/24;
//}

//void SoundComposition::generateData() {
//    if(mSounds.isEmpty()) {
//        mBuffer.resize(0);
//        return;
//    }
//    const int channelBytes = 2;
//    //const int sampleBytes = format.channelCount() * channelBytes;

//    qint64 nAudioFrames = mLastAudioFrame - mFirstAudioFrame;
//    qint64 length = 2 * 2 * nAudioFrames;
//    mBuffer.resize(length);
//    unsigned char *ptr = reinterpret_cast<unsigned char *>(mBuffer.data());
//    for(int i = 0; i < nAudioFrames; i++) {
//        qToLittleEndian<quint16>(10000, ptr);
//        ptr += channelBytes;
//        qToLittleEndian<quint16>(10000, ptr);
//        ptr += channelBytes;
//    }

//    foreach(SingleSound *sound, mSounds) {
//        qint64 startAudioFrame = sound->getStartAudioFrame();
//        qint64 endAudioFrame = startAudioFrame + sound->getAudioFrameLength();
//        if(startAudioFrame > mLastAudioFrame ||
//           endAudioFrame < mFirstAudioFrame ||
//           startAudioFrame == endAudioFrame) {
//            continue;
//        }
//        const unsigned char *soundData = sound->getConstData();
//        int iMin = qMax((qint64)0,
//                        mFirstAudioFrame - startAudioFrame);
//        int iMax = qMin(endAudioFrame - startAudioFrame,
//                        mLastAudioFrame - startAudioFrame);
//        unsigned char *ptrT = ptr + startAudioFrame*channelBytes;
//        for(int i = iMin; i <= iMax; i++) {
//            quint16 valueLeft = static_cast<quint16>(*soundData);
//            soundData += channelBytes;
//            qToLittleEndian<quint16>(valueLeft, ptrT);
//            ptrT += channelBytes;

//            quint16 valueRight = static_cast<quint16>(*soundData);
//            soundData += channelBytes;
//            qToLittleEndian<quint16>(valueRight, ptrT);
//            ptrT += channelBytes;
//        }
//    }
//}

//SoundComposition::~SoundComposition() {

//}

//void SoundComposition::start() {
//    open(QIODevice::ReadOnly);
//}

//void SoundComposition::stop() {
//    mPos = 0;
//    close();
//}

//qint64 SoundComposition::readData(char *data, qint64 len)
//{
//    qint64 total = 0;
//    if (!mBuffer.isEmpty()) {
//        while(len - total > 0) {
//            const qint64 chunk = qMin((mBuffer.size() - mPos), len - total);
//            memcpy(data + total, mBuffer.constData() + mPos, chunk);
//            mPos = (mPos + chunk) % mBuffer.size();
//            total += chunk;
//        }
//    }
//    return total;
//}

//qint64 SoundComposition::writeData(const char *data, qint64 len) {
//    Q_UNUSED(data);
//    Q_UNUSED(len);

//    return 0;
//}

//qint64 SoundComposition::bytesAvailable() const {
//    return mBuffer.size() + QIODevice::bytesAvailable();
//}

//void SoundComposition::addSound(SingleSound *sound) {
//    mSounds << sound;
//}

//void SoundComposition::removeSound(SingleSound *sound) {
//    mSounds.removeOne(sound);
//}
