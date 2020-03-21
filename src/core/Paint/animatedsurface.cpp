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

#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface() : Animator("canvas"),
    mBaseValue(enve::make_shared<DrawableAutoTiledSurface>()),
    mCurrent_d(mBaseValue.get()) {
    connect(this, &Animator::anim_addedKey,
            this, [this](Key* const key) {
        updateCurrent();
        if(!mUseRange.inRange(key->getRelFrame())) return;
        const auto asKey = static_cast<ASKey*>(key);
        const auto dSurf = asKey->dSurface();
        if(!dSurf.storesDataInMemory())
            asKey->dSurface().scheduleLoadFromTmpFile();
        mUsed.append(&asKey->dSurface());
    });
    connect(this, &Animator::anim_removedKey,
            this, [this](Key* const key) {
        updateCurrent();
        if(!anim_hasKeys()) {
            *mBaseValue.get() = static_cast<ASKey*>(key)->dSurface();
        }
        if(!mUseRange.inRange(key->getRelFrame())) return;
        const auto asKey = static_cast<ASKey*>(key);
        mUsed.removeOne(&asKey->dSurface());
    });
}

void AnimatedSurface::prp_afterChangedAbsRange(const FrameRange &range, const bool clip) {
    Animator::prp_afterChangedAbsRange(range, clip);
    const auto relRange = prp_absRangeToRelRange(range);
    mFrameImagesCache.remove(relRange);
}

void AnimatedSurface::afterSurfaceChanged(DrawableAutoTiledSurface * const surface) {
    if(!anim_hasKeys() && mCurrent_d == surface)
        prp_afterWholeInfluenceRangeChanged();
    for(const auto& key : anim_getKeys()) {
        const auto& asKey = static_cast<ASKey*>(key);
        if(&asKey->dSurface() != surface) continue;
        anim_updateAfterChangedKey(key);
        break;
    }
}

template <typename T>
void AnimatedSurface::loadPixmapT(const T &src) {
    const bool createNewFrame = anim_isRecording() &&
            !anim_getKeyOnCurrentFrame();
    if(createNewFrame) newEmptyFrame();
    auto& target = mCurrent_d->surface();
    const bool undoRedo = getParentScene();
    if(undoRedo) {
        target.triggerAllChange();
        const auto roi = mCurrent_d->pixelBoundingRect();
        mCurrent_d->loadPixmap(src);
        addUndoRedo("Load Image", roi);
    } else mCurrent_d->loadPixmap(src);
    afterChangedCurrentContent();
}

void AnimatedSurface::loadPixmap(const QImage &src) {
    loadPixmapT(src);
}

void AnimatedSurface::loadPixmap(const sk_sp<SkImage> &src) {
    if(!src) return;
    SkPixmap pixmap;
    const auto raster = src->makeRasterImage();
    if(raster->peekPixels(&pixmap)) loadPixmap(pixmap);
}

void AnimatedSurface::loadPixmap(const SkPixmap &src) {
    loadPixmapT(src);
}

void AnimatedSurface::afterChangedCurrentContent() {
    const int relFrame = anim_getCurrentRelFrame();
    const auto identicalRange = prp_getIdenticalRelRange(relFrame);
    prp_afterChangedRelRange(identicalRange);
}

void AnimatedSurface::addUndoRedo(const QString& name, const QRect& roi) {
    auto& target = mCurrent_d->surface();
    auto undoList = target.takeUndoList();
    if(undoList.isEmpty()) return;
    {
        prp_pushUndoRedoName(name);
        const stdptr<DrawableAutoTiledSurface> ptr = mCurrent_d;
        UndoRedo ur;

        const auto replaceTile = [this, undoList, ptr, roi](
                const stdsptr<Tile>& (UndoTile::*getter)() const) {
            if(!ptr) return;
            auto& surface = ptr->surface();
            for(const auto& undoTile : undoList) {
                surface.replaceTile(undoTile.tileX(),
                                    undoTile.tileY(),
                                    (undoTile.*getter)());
            }
            surface.autoCrop();
            ptr->updateTileDimensions();
            ptr->pixelRectChanged(roi);
            afterChangedCurrentContent();
        };

        ur.fUndo = [replaceTile]() {
            replaceTile(&UndoTile::oldValue);
        };
        ur.fRedo = [replaceTile]() {
            replaceTile(&UndoTile::newValue);
        };
        prp_addUndoRedo(ur);
    }
}

#include "svgexporthelpers.h"
#include "Private/Tasks/taskscheduler.h"

bool AnimatedSurface::savePaintSVG(SvgExporter& exp, QDomElement& parent) {
    const auto relRange = prp_absRangeToRelRange(exp.fAbsRange);
    const auto idRange = prp_getIdenticalRelRange(relRange.fMin);
    const int span = exp.fAbsRange.span();
    const qreal div = span - 1;
    const qreal dur = div/exp.fFps;
    const auto taskScheduler = TaskScheduler::instance();

    auto use = exp.createElement("use");

    QStringList hrefValues;
    QStringList xValues;
    QStringList yValues;
    QStringList keyTimes;

    const auto useCreator = [&](const int relFrame,
                                DrawableAutoTiledSurface* surf) {
        if(!surf) surf = getSurface(relFrame);
        sk_sp<SkImage> image;
        const auto task = getFrameImage(relFrame, image);
        if(task) taskScheduler->waitTillFinished();

        const qreal t = (relFrame - relRange.fMin)/div;
        keyTimes << QString::number(t);

        const QString imageId = SvgExportHelpers::ptrToStr(surf);
        SvgExportHelpers::defImage(exp, image, imageId);
        hrefValues << "#" + imageId;

        const QPoint pos = -surf->zeroTilePos();
        xValues << QString::number(pos.x());
        yValues << QString::number(pos.y());
    };

    if(idRange.inRange(relRange) || span == 1) {
        useCreator(relRange.fMin, nullptr);
    } else {
        ASKey* prevKey = nullptr;

        const auto& keys = anim_getKeys();
        bool first = true;
        for(const auto &i : keys) {
            const auto key = static_cast<ASKey*>(i);
            const int keyRelFrame = key->getRelFrame();
            if(keyRelFrame >= relRange.fMax) break;
            if(keyRelFrame >= relRange.fMin) {
                const bool keyOnFirstFrame = keyRelFrame == relRange.fMin;
                if(first) {
                    first = false;
                    const auto firstKey = keyOnFirstFrame ? key : prevKey;
                    useCreator(keyRelFrame, &firstKey->dSurface());
                } else useCreator(keyRelFrame, &key->dSurface());
            }
            prevKey = key;
        }
    }

    if(hrefValues.isEmpty()) return false;

    hrefValues << hrefValues.last();
    xValues << xValues.last();
    yValues << yValues.last();
    keyTimes << "1";

    const auto durStr = QString::number(dur)  + 's';
    const auto keyTimesStr = keyTimes.join(';');
    {
        auto anim = exp.createElement("animate");
        anim.setAttribute("attributeName", "href");
        anim.setAttribute("dur", durStr);
        anim.setAttribute("values", hrefValues.join(';'));
        anim.setAttribute("keyTimes", keyTimesStr);
        SvgExportHelpers::assignLoop(anim, exp.fLoop);
        use.appendChild(anim);
    }
    {
        auto anim = exp.createElement("animate");
        anim.setAttribute("attributeName", "x");
        anim.setAttribute("dur", durStr);
        anim.setAttribute("values", xValues.join(';'));
        anim.setAttribute("keyTimes", keyTimesStr);
        SvgExportHelpers::assignLoop(anim, exp.fLoop);
        use.appendChild(anim);
    }
    {
        auto anim = exp.createElement("animate");
        anim.setAttribute("attributeName", "y");
        anim.setAttribute("dur", durStr);
        anim.setAttribute("values", yValues.join(';'));
        anim.setAttribute("keyTimes", keyTimesStr);
        SvgExportHelpers::assignLoop(anim, exp.fLoop);
        use.appendChild(anim);
    }

    parent.appendChild(use);
    return true;
}

ASKey::ASKey(AnimatedSurface * const parent) :
    Key(parent),
    mValue(enve::make_shared<DrawableAutoTiledSurface>()) {}

ASKey::ASKey(const int frame, AnimatedSurface * const parent) :
    Key(frame, parent),
    mValue(enve::make_shared<DrawableAutoTiledSurface>()) {}

ASKey::ASKey(const DrawableAutoTiledSurface &value,
             const int frame, AnimatedSurface * const parent) :
    ASKey(frame, parent) {
    *mValue.get() = value;
}
