#include "singlesound.h"
#include "soundcomposition.h"
#include "durationrectangle.h"
#include "PropertyUpdaters/singlesoundupdater.h"
#include "filesourcescache.h"
#include "Decode/audiodecode.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"

SingleSound::SingleSound(const qsptr<FixedLenAnimationRect>& durRect) :
    ComplexAnimator("sound") {
    setDurationRect(durRect);

    prp_setOwnUpdater(SPtrCreate(SingleSoundUpdater)(this));

    ca_addChildAnimator(mVolumeAnimator);
}

DurationRectangleMovable *SingleSound::anim_getTimelineMovable(
        const int &relX, const int &minViewedFrame,
        const qreal &pixelsPerFrame) {
    if(!mDurationRectangle) return nullptr;
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void SingleSound::drawTimelineControls(QPainter * const p,
                                       const qreal &pixelsPerFrame,
                                       const FrameRange &absFrameRange,
                                       const int &rowHeight) {
//    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
//    int startDFrame = mDurationRectangle.getMinAnimationFrame() - startFrame;
//    int frameWidth = ceil(mListOfFrames.count()/qAbs(timeScale));
//    p->fillRect(startDFrame*pixelsPerFrame + pixelsPerFrame*0.5, drawY,
//                frameWidth*pixelsPerFrame - pixelsPerFrame,
//                BOX_HEIGHT, QColor(0, 0, 255, 125));
    if(mDurationRectangle) {
        p->save();
        if(mOwnDurationRectangle)
            p->translate(prp_getParentFrameShift()*pixelsPerFrame, 0);
        const int width = qFloor(absFrameRange.span()*pixelsPerFrame);
        const QRect drawRect(0, 0, width, rowHeight);
        mDurationRectangle->draw(p, drawRect, getCanvasFPS(),
                                 pixelsPerFrame, absFrameRange);
        p->restore();
    }
    ComplexAnimator::drawTimelineControls(p, pixelsPerFrame,
                                          absFrameRange, rowHeight);
}

FixedLenAnimationRect *SingleSound::getDurationRect() const {
    return mDurationRectangle.get();
}

#include "canvas.h"

int SingleSound::getSampleShift() const{
    const qreal fps = getCanvasFPS();
    return qRound(prp_getFrameShift()*(SOUND_SAMPLERATE/fps));
}

SampleRange SingleSound::relSampleRange() const {
    const qreal fps = getCanvasFPS();
    const auto durRect = getDurationRect();
    const auto relFrameRange = durRect->getRelFrameRange();
    const auto qRelFrameRange = qValueRange{qreal(relFrameRange.fMin),
                                            qreal(relFrameRange.fMax)};
    const auto qSampleRange = qRelFrameRange*(SOUND_SAMPLERATE/fps);
    return {qFloor(qSampleRange.fMin), qCeil(qSampleRange.fMax)};
}

SampleRange SingleSound::absSampleRange() const {
    const qreal fps = getCanvasFPS();
    const auto durRect = getDurationRect();
    const auto absFrameRange = durRect->getAbsFrameRange();
    const auto qAbsFrameRange = qValueRange{qreal(absFrameRange.fMin),
                                            qreal(absFrameRange.fMax)};
    const auto qSampleRange = qAbsFrameRange*(SOUND_SAMPLERATE/fps);
    return {qFloor(qSampleRange.fMin), qCeil(qSampleRange.fMax)};
}

iValueRange SingleSound::absSecondToRelSeconds(const int &absSecond) {
    const qreal fps = getCanvasFPS();
    const qreal qFirstSecond = prp_absFrameToRelFrameF(absSecond*fps)/fps;
    if(isInteger4Dec(qFirstSecond)) {
        const int round = qRound(qFirstSecond);
        return {round, round};
    }
    const int firstSecond = qFloor(qFirstSecond);
    const int lastSecond = qCeil(qFirstSecond);
    return {firstSecond, lastSecond};
}

qreal SingleSound::getCanvasFPS() const {
    auto parentCanvas = getFirstAncestor<Canvas>();
    if(!parentCanvas) {
        const auto box = getFirstAncestor<BoundingBox>();
        if(!box) return 1;
        parentCanvas = box->getParentCanvas();
        if(!parentCanvas) return 1;
    }
    return parentCanvas->getFps();
}

void SingleSound::setDurationRect(const qsptr<FixedLenAnimationRect>& durRect) {
    if(mDurationRectangle) mDurationRectangle->setSoundCacheHandler(nullptr);
    if(!durRect) {
        mOwnDurationRectangle = true;
        mDurationRectangle = SPtrCreate(FixedLenAnimationRect)(this);
        mDurationRectangle->setBindToAnimationFrameRange();
        connect(mDurationRectangle.get(), &DurationRectangle::posChanged,
                this, &SingleSound::anim_updateAfterShifted);
    } else {
        mOwnDurationRectangle = false;
        mDurationRectangle = durRect;
    }
    connect(mDurationRectangle.get(), &DurationRectangle::rangeChanged,
            this, &SingleSound::scheduleFinalDataUpdate);
    connect(mDurationRectangle.get(), &DurationRectangle::posChanged,
            this, &SingleSound::updateAfterDurationRectangleShifted);
    mDurationRectangle->setSoundCacheHandler(&getCacheHandler());
}

void SingleSound::updateAfterDurationRectangleShifted() {
    prp_afterFrameShiftChanged();
    anim_setAbsFrame(anim_getCurrentAbsFrame());
    scheduleFinalDataUpdate();
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
    mCacheHandler->setFilePath(path);
    if(mOwnDurationRectangle) {
        const int secs = mCacheHandler->durationSec();
        const qreal fps = getCanvasFPS();
        const int frames = qCeil(secs*fps);
        mDurationRectangle->setAnimationFrameDuration(frames);
    }
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
        mFinalData = new float[static_cast<size_t>(mFinalSampleCount)];
        if(mVolumeAnimator->anim_hasKeys()) {
            int j = 0;
            int frame = 0;
            qreal volVal = mVolumeAnimator->getEffectiveValue(frame);
            float lastFrameVol = static_cast<float>(volVal*0.01);
            const float volStep = fps/SOUND_SAMPLERATE;
            while(j < mFinalSampleCount) {
                frame++;
                float nextFrameVol = static_cast<float>(
                        mVolumeAnimator->getEffectiveValue(frame)/100.);
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
                    static_cast<float>(mVolumeAnimator->getCurrentBaseValue()/100.);
            for(int i = 0; i < mFinalSampleCount; i++) {
                mFinalData[i] = mSrcData[i + minSampleFromSrc]*volFrac;
            }
        }
    }
}

int SingleSound::prp_getRelFrameShift() const {
    if(mOwnDurationRectangle)
        return mDurationRectangle->getFrameShift();
    return 0;
}

bool SingleSound::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool &parentSatisfies,
                                      const bool &parentMainTarget) const {
    if(rules.fType == SWT_TYPE_SOUND) return true;
    if(rules.fType == SWT_TYPE_GRAPHICS) return false;
    return SingleWidgetTarget::SWT_shouldBeVisible(rules,
                                                   parentSatisfies,
                                                   parentMainTarget);
}

FrameRange SingleSound::prp_relInfluenceRange() const {
    return mDurationRectangle->getAbsFrameRange();
}

const float *SingleSound::getFinalData() const {
    return mFinalData;
}
