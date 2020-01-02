// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "singlesound.h"
#include "soundcomposition.h"
#include "Timeline/fixedlenanimationrect.h"
#include "filesourcescache.h"
#include "CacheHandlers/soundcachehandler.h"
#include "fileshandler.h"
#include "esoundlink.h"
#include "../canvas.h"

SingleSound::SingleSound(const qsptr<FixedLenAnimationRect>& durRect) :
    mIndependent(!durRect) {
    connect(this, &eBoxOrSound::prp_ancestorChanged, this, [this]() {
        if(!getParentScene()) return;
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
void SingleSound::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    const PropertyMenu::CheckSelectedOp<SingleSound> enableOp =
            [](SingleSound * sound, bool enable) {
        sound->setVisibile(enable);
    };
    menu->addCheckableAction("Enabled", isVisible(), enableOp);
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

    const PropertyMenu::PlainTriggeredOp deleteOp = [this]() {
        removeFromParent_k();
    };
    menu->addPlainAction("Delete", deleteOp);
}

SoundReaderForMerger *SingleSound::getSecondReader(const int relSecondId) {
    const int maxSec = mCacheHandler->durationSecCeil() - 1;
    if(relSecondId < 0 || relSecondId > maxSec) return nullptr;
    const auto reader = mCacheHandler->getSecondReader(relSecondId);
    if(!reader) return mCacheHandler->addSecondReader(relSecondId);
    return reader;
}

stdsptr<Samples> SingleSound::getSamplesForSecond(const int relSecondId) {
    return mCacheHandler->getSamplesForSecond(relSecondId);
}

int SingleSound::durationSeconds() const {
    return mCacheHandler ? mCacheHandler->durationSecCeil() : 0;
}

qsptr<eSound> SingleSound::createLink() {
    return enve::make_shared<eSoundLink>(this);
}

#include "canvas.h"

const HddCachableCacheHandler *SingleSound::getCacheHandler() const {
    if(!mCacheHandler) return nullptr;
    return &mCacheHandler->getCacheHandler();
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
    if(mIndependent && mCacheHandler && getParentScene()) {
        const qreal secs = mCacheHandler ? mCacheHandler->durationSec() : 0;
        const qreal fps = getCanvasFPS();
        const int frames = qCeil(qAbs(secs*fps*mStretch));
        const auto flaRect = static_cast<FixedLenAnimationRect*>(
                    mDurationRectangle.get());
        flaRect->setAnimationFrameDuration(frames);
    }
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

#include "ReadWrite/basicreadwrite.h"
void SingleSound::prp_writeProperty(eWriteStream& dst) const {
    if(videoSound()) {
        StaticComplexAnimator::prp_writeProperty(dst);
        dst << mVisible;
        return;
    }
    eBoxOrSound::prp_writeProperty(dst);
    const auto filePath = mCacheHandler ? mCacheHandler->getFilePath() : "";
    dst << filePath;
}

void SingleSound::prp_readProperty(eReadStream& src) {
    if(videoSound()) {
        StaticComplexAnimator::prp_readProperty(src);
        src >> mVisible;
        return;
    }
    eBoxOrSound::prp_readProperty(src);
    QString filePath;
    src >> filePath;
    if(!filePath.isEmpty()) setFilePath(filePath);
}
