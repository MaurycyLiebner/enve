#include "singlesound.h"
#include "soundcomposition.h"
#include "Timeline/fixedlenanimationrect.h"
#include "filesourcescache.h"
#include "CacheHandlers/soundcachehandler.h"
#include "fileshandler.h"
#include "../canvas.h"

SingleSound::SingleSound(const qsptr<FixedLenAnimationRect>& durRect) :
    eBoxOrSound("sound"), mIndependent(!durRect) {
    connect(this, &eBoxOrSound::aboutToChangeAncestor, this, [this]() {
        if(!mParentScene) return;
        mParentScene->getSoundComposition()->removeSound(ref<SingleSound>());
    });
    connect(this, &eBoxOrSound::ancestorChanged, this, [this]() {
        if(!mParentScene) return;
        mParentScene->getSoundComposition()->addSound(ref<SingleSound>());
        updateDurationRectLength();
    });
    if(mIndependent) {
        const auto flaRect = enve::make_shared<FixedLenAnimationRect>(*this);
        flaRect->setBindToAnimationFrameRange();
        setDurationRectangle(flaRect);
    } else setDurationRectangle(durRect);
    mDurationRectangleLocked = true;

    mDurationRectangle->setSoundCacheHandler(getCacheHandler());

    ca_addChild(mVolumeAnimator);
}

#include <QInputDialog>
#include "typemenu.h"
void SingleSound::setupTreeViewMenu(PropertyMenu * const menu) {
    const PropertyMenu::CheckSelectedOp<SingleSound> enableOp =
            [](SingleSound * sound, bool enable) {
        sound->setEnabled(enable);
    };
    menu->addCheckableAction("Enabled", isEnabled(), enableOp);
    if(videoSound()) return;
    const auto widget = menu->getParentWidget();
    const PropertyMenu::PlainSelectedOp<SingleSound> stretchOp =
            [this, widget](SingleSound * sound) {
        bool ok = false;
        const int stretch = QInputDialog::getInt(widget,
                                                 "Stretch " + sound->prp_getName(),
                                                 "Stretch:",
                                                 qRound(getStretch()*100),
                                                 -1000, 1000, 1, &ok);
        if(!ok) return;
        sound->setStretch(stretch*0.01);
    };
    menu->addPlainAction("Stretch...", stretchOp);
}

SoundReaderForMerger *SingleSound::getSecondReader(const int relSecondId) {
    const int maxSec = mCacheHandler->durationSec() - 1;
    if(relSecondId < 0 || relSecondId > maxSec) return nullptr;
    const auto reader = mCacheHandler->getSecondReader(relSecondId);
    if(!reader) return mCacheHandler->addSecondReader(relSecondId);
    return reader;
}

stdsptr<Samples> SingleSound::getSamplesForSecond(const int relSecondId) {
    return mCacheHandler->getSamplesForSecond(relSecondId);
}

#include "canvas.h"

int SingleSound::getSampleShift() const{
    const qreal fps = getCanvasFPS();
    return qRound(prp_getTotalFrameShift()*(eSoundSettings::sSampleRate()/fps));
}

SampleRange SingleSound::relSampleRange() const {
    const qreal fps = getCanvasFPS();
    const auto durRect = getDurationRectangle();
    const auto relFrameRange = durRect->getRelFrameRange();
    const auto qRelFrameRange = qValueRange{qreal(relFrameRange.fMin),
                                            qreal(relFrameRange.fMax)};
    const auto qSampleRange = qRelFrameRange*(eSoundSettings::sSampleRate()/fps);
    return {qFloor(qSampleRange.fMin), qCeil(qSampleRange.fMax)};
}

SampleRange SingleSound::absSampleRange() const {
    const qreal fps = getCanvasFPS();
    const auto durRect = getDurationRectangle();
    const auto absFrameRange = durRect->getAbsFrameRange();
    const auto qAbsFrameRange = qValueRange{qreal(absFrameRange.fMin),
                                            qreal(absFrameRange.fMax)};
    const auto qSampleRange = qAbsFrameRange*(eSoundSettings::sSampleRate()/fps);
    return {qFloor(qSampleRange.fMin), qCeil(qSampleRange.fMax)};
}

iValueRange SingleSound::absSecondToRelSeconds(const int absSecond) {
    if(mStretch < 0) {
        const auto absStretch = absSecondToRelSecondsAbsStretch(absSecond);
        const int secs = mCacheHandler ? mCacheHandler->durationSec() : 0;
        return {secs - absStretch.fMax, secs - absStretch.fMin};
    }
    return absSecondToRelSecondsAbsStretch(absSecond);
}

const HDDCachableCacheHandler *SingleSound::getCacheHandler() const {
    if(!mCacheHandler) return nullptr;
    return &mCacheHandler->getCacheHandler();
}

iValueRange SingleSound::absSecondToRelSecondsAbsStretch(const int absSecond) {
    const qreal fps = getCanvasFPS();
    const qreal stretch = qAbs(mStretch);
    const qreal speed = isZero6Dec(stretch) ? TEN_MIL : 1/stretch;
    const qreal qFirstSecond = prp_absFrameToRelFrameF(absSecond*fps)*speed/fps;
    if(isInteger4Dec(qFirstSecond)) {
        const int round = qRound(qFirstSecond);
        if(isOne4Dec(stretch) || stretch > 1) {
            return {round, round};
        }
        const qreal qLast = qFirstSecond + speed - 1;
        if(isInteger4Dec(qLast)) {
            const int roundLast = qRound(qLast);
            return {round, roundLast};
        }
        const int ceilLast = qMax(round, qCeil(qLast));
        return {round, ceilLast};
    }
    const int firstSecond = qFloor(qFirstSecond);
    const int lastSecond = qCeil(qFirstSecond + speed - 1);
    return {firstSecond, lastSecond};
}

void SingleSound::setStretch(const qreal stretch) {
    mStretch = stretch;
    updateDurationRectLength();
    prp_afterWholeInfluenceRangeChanged();
}

QrealSnapshot SingleSound::getVolumeSnap() const {
    return mVolumeAnimator->makeSnapshot(
                eSoundSettings::sSampleRate()/getCanvasFPS(), 0.01);
}

void SingleSound::updateDurationRectLength() {
    if(mIndependent && mCacheHandler && mParentScene) {
        const int secs = mCacheHandler ? mCacheHandler->durationSec() : 0;
        const qreal fps = getCanvasFPS();
        const int frames = qCeil(qAbs(secs*fps*mStretch));
        const auto flaRect = static_cast<FixedLenAnimationRect*>(
                    mDurationRectangle.get());
        flaRect->setAnimationFrameDuration(frames);
    }
}

qreal SingleSound::getCanvasFPS() const {
    auto parentCanvas = getFirstAncestor<Canvas>();
    if(!parentCanvas) {
        const auto box = getFirstAncestor<BoundingBox>();
        if(!box) return 1;
        parentCanvas = box->getParentScene();
        if(!parentCanvas) return 1;
    }
    return parentCanvas->getFps();
}

void SingleSound::setFilePath(const QString &path) {
    FilesHandler::sInstance->getFileHandler<SoundFileHandler>(path);
    if(videoSound()) RuntimeThrow("Setting file path for video sound");
    const auto newDataHandler = FileDataCacheHandler::
            sGetDataHandler<SoundDataHandler>(path);
    setSoundDataHandler(newDataHandler);
    prp_setName(QFileInfo(path).fileName());
}

void SingleSound::setSoundDataHandler(SoundDataHandler* const newDataHandler) {
    mCacheHandler.reset();
    if(newDataHandler) mCacheHandler = enve::make_shared<SoundHandler>(newDataHandler);
    mDurationRectangle->setSoundCacheHandler(getCacheHandler());
    updateDurationRectLength();
}

int SingleSound::prp_getRelFrameShift() const {
    if(mIndependent) return eBoxOrSound::prp_getRelFrameShift();
    return 0;
}

bool SingleSound::SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                      const bool parentSatisfies,
                                      const bool parentMainTarget) const {
    if(mIndependent) {
        if(rules.fRule == SWT_BR_VISIBLE && !mVisible) return false;
        if(rules.fRule == SWT_BR_SELECTED && !mSelected) return false;
        if(rules.fType == SWT_TYPE_SOUND) return true;
        if(rules.fType == SWT_TYPE_GRAPHICS) return false;
        return parentSatisfies;
    } else {
        return StaticComplexAnimator::SWT_shouldBeVisible(
                    rules, parentSatisfies, parentMainTarget);
    }
}

#include "basicreadwrite.h"
void SingleSound::writeProperty(QIODevice * const dst) const {
    if(videoSound()) return StaticComplexAnimator::writeProperty(dst);
    eBoxOrSound::writeProperty(dst);
    const auto filePath = mCacheHandler ? mCacheHandler->getFilePath() : "";
    gWrite(dst, filePath);
}

void SingleSound::readProperty(QIODevice * const src) {
    if(videoSound()) return StaticComplexAnimator::readProperty(src);
    eBoxOrSound::readProperty(src);
    const QString filePath = gReadString(src);
    if(!filePath.isEmpty()) setFilePath(filePath);
}
