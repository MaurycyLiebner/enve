#include "singlesound.h"
#include "soundcomposition.h"
#include "durationrectangle.h"
#include "PropertyUpdaters/singlesoundupdater.h"
#include "filesourcescache.h"
#include "Decode/audiodecode.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"

SingleSound::SingleSound(const QString &path,
                         FixedLenAnimationRect *durRect) :
    ComplexAnimator("sound") {
    setDurationRect(durRect);

    prp_setUpdater(SPtrCreate(SingleSoundUpdater)(this));
    prp_blockUpdater();

    ca_addChildAnimator(mVolumeAnimator);

    setFilePath(path);
}

void SingleSound::prp_drawKeys(QPainter *p,
                               const qreal &pixelsPerFrame,
                               const qreal &drawY,
                               const int &startFrame,
                               const int &endFrame,
                               const int &rowHeight,
                               const int &keyRectSize) {
//    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
//    int startDFrame = mDurationRectangle.getMinAnimationFrame() - startFrame;
//    int frameWidth = ceil(mListOfFrames.count()/qAbs(timeScale));
//    p->fillRect(startDFrame*pixelsPerFrame + pixelsPerFrame*0.5, drawY,
//                frameWidth*pixelsPerFrame - pixelsPerFrame,
//                BOX_HEIGHT, QColor(0, 0, 255, 125));
    mDurationRectangle->draw(p, pixelsPerFrame,
                             drawY, startFrame);
    ComplexAnimator::prp_drawKeys(p, pixelsPerFrame, drawY,
                                  startFrame, endFrame,
                                  rowHeight, keyRectSize);
}

FixedLenAnimationRect *SingleSound::getDurationRect() {
    return mDurationRectangle;
}

void SingleSound::setDurationRect(FixedLenAnimationRect *durRect) {
    if(mDurationRectangle) {
        delete mDurationRectangle;
    }
    if(!durRect) {
        mOwnDurationRectangle = true;
        mDurationRectangle = new FixedLenAnimationRect(this);
        mDurationRectangle->setBindToAnimationFrameRange();
        connect(mDurationRectangle, &DurationRectangle::posChanged,
                this, &SingleSound::anim_updateAfterShifted);
    } else {
        mOwnDurationRectangle = false;
        mDurationRectangle = durRect;
    }
    connect(mDurationRectangle, &DurationRectangle::rangeChanged,
            this, &SingleSound::scheduleFinalDataUpdate);
    connect(mDurationRectangle, &DurationRectangle::posChanged,
            this, &SingleSound::updateAfterDurationRectangleShifted);
}

void SingleSound::updateAfterDurationRectangleShifted() {
    prp_setParentFrameShift(prp_mParentFrameShift);
    prp_setAbsFrame(anim_mCurrentAbsFrame);
    scheduleFinalDataUpdate();
}

DurationRectangleMovable *SingleSound::anim_getRectangleMovableAtPos(
                            const int &relX,
                            const int &minViewedFrame,
                            const qreal &pixelsPerFrame) {
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void SingleSound::updateFinalDataIfNeeded(const qreal &fps,
                                          const int &minAbsFrame,
                                          const int &maxAbsFrame) {
    if(mFinalDataUpdateNeeded) {
        prepareFinalData(static_cast<float>(fps), minAbsFrame, maxAbsFrame);
        mFinalDataUpdateNeeded = false;
    }
}

void SingleSound::scheduleFinalDataUpdate() {
    mFinalDataUpdateNeeded = true;
}


void SingleSound::setFilePath(const QString &path) {
    mPath = path;
    reloadDataFromFile();
}

void SingleSound::reloadDataFromFile() {
    if(mSrcData) {
        free(mSrcData);
        mSrcData = nullptr;
        mSrcSampleCount = 0;
    }
    if(!mPath.isEmpty()) {
        if(QFile(mPath).exists()) {
            SampleRange dataRange{0, 10*SOUND_SAMPLERATE};
            gDecodeSoundDataRange(mPath.toLatin1().data(),
                                  dataRange, mSrcData);
            mSrcSampleCount = dataRange.fMax - dataRange.fMin + 1;
//            decode_audio_file(mPath.toLatin1().data(), SOUND_SAMPLERATE,
//                              &mSrcData, &mSrcSampleCount);
        }
    }
    if(mOwnDurationRectangle) {
        mDurationRectangle->setAnimationFrameDuration(
                    qCeil(mSrcSampleCount*24./SOUND_SAMPLERATE));
    }

    scheduleFinalDataUpdate();
}

int SingleSound::getStartAbsFrame() const {
    return mFinalAbsStartFrame;
}

int SingleSound::getSampleCount() const {
    return mFinalSampleCount;
}

void SingleSound::prepareFinalData(const float &fps,
                                   const int &minAbsFrame,
                                   const int &maxAbsFrame) {
    if(mFinalData) free(mFinalData);
    if(!mSrcData) {
        mFinalData = nullptr;
        mFinalSampleCount = 0;
    } else {
        mFinalAbsStartFrame =
            qMax(minAbsFrame,
                   qMax(mDurationRectangle->getMinFrameAsAbsFrame(),
                        mDurationRectangle->getMinAnimationFrameAsAbsFrame()) );
        int finalMaxAbsFrame =
            qMin(maxAbsFrame,
                   qMin(mDurationRectangle->getMaxFrameAsAbsFrame(),
                        mDurationRectangle->getMaxAnimationFrameAsAbsFrame()) );
        qDebug() << "sound abs frame range:" << mFinalAbsStartFrame << finalMaxAbsFrame;
        int finalMinRelFrame = prp_absFrameToRelFrame(mFinalAbsStartFrame);
        int finalMaxRelFrame = prp_absFrameToRelFrame(finalMaxAbsFrame);
        qDebug() << "sound rel frame range:" << finalMinRelFrame << finalMaxRelFrame;

        int minSampleFromSrc = static_cast<int>(
                    floor(finalMinRelFrame*SOUND_SAMPLERATE/fps));
        int maxSamplePlayed = static_cast<int>(
                    ceil(finalMaxRelFrame*SOUND_SAMPLERATE/fps));
        int maxSampleFromSrc = std::min(mSrcSampleCount, maxSamplePlayed);


        mFinalSampleCount = static_cast<int>(maxSampleFromSrc - minSampleFromSrc);
        qDebug() << minSampleFromSrc << maxSampleFromSrc << mFinalSampleCount;
        ulong dataSize = static_cast<ulong>(mFinalSampleCount)*sizeof(float);
        void* data = malloc(dataSize);
        mFinalData = static_cast<float*>(data);
        if(mVolumeAnimator->prp_hasKeys()) {
            int j = 0;
            int frame = 0;
            qreal volVal = mVolumeAnimator->qra_getEffectiveValueAtRelFrame(frame);
            float lastFrameVol = static_cast<float>(volVal*0.01);
            const float volStep = fps/SOUND_SAMPLERATE;
            while(j < mFinalSampleCount) {
                frame++;
                float nextFrameVol = static_cast<float>(
                        mVolumeAnimator->qra_getEffectiveValueAtRelFrame(frame)/100.);
                float volDiff = (nextFrameVol - lastFrameVol);
                float currVolFrac = lastFrameVol;
                for(int i = 0;
                    i < SOUND_SAMPLERATE/fps && j < mFinalSampleCount;
                    i++, j++) {
                    currVolFrac += volStep*volDiff;
                    mFinalData[j] = static_cast<float>(mSrcData[j + minSampleFromSrc]*
                                            currVolFrac);
                }
                lastFrameVol = nextFrameVol;
            }
        } else {
            const float volFrac =
                    static_cast<float>(mVolumeAnimator->qra_getCurrentValue()/100.);
            for(int i = 0; i < mFinalSampleCount; i++) {
                mFinalData[i] = mSrcData[i + minSampleFromSrc]*volFrac;
            }
        }
    }
}

int SingleSound::prp_getFrameShift() const {
    if(mOwnDurationRectangle) {
        return mDurationRectangle->getFrameShift() +
                Animator::prp_getFrameShift();
    }
    return Animator::prp_getFrameShift();
}

bool SingleSound::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &parentMainTarget) const {
    if(rules.type == &SingleWidgetTarget::SWT_isSingleSound) return true;
    return SingleWidgetTarget::SWT_shouldBeVisible(rules,
                                                   parentSatisfies,
                                                   parentMainTarget);
}

const float *SingleSound::getFinalData() const {
    return mFinalData;
}
