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

#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "Animators/graphanimator.h"
#include "Animators/interpolationkeyt.h"
#include "drawableautotiledsurface.h"
#include "Animators/qrealpoint.h"
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/imagecachecontainer.h"
#include "onionskin.h"

class AnimatedSurface;
class ComplexTask;
class DomEleTask;

class CORE_EXPORT ASKey : public Key {
    e_OBJECT
protected:
    ASKey(AnimatedSurface * const parent);
    ASKey(const int frame, AnimatedSurface * const parent);
    ASKey(const DrawableAutoTiledSurface &value,
          const int frame, AnimatedSurface * const parent);
public:
    bool differsFromKey(Key * const key) const {
        return key != this;
    }

    void writeKey(eWriteStream& dst) {
        Key::writeKey(dst);
        mValue->write(dst);
    }

    void readKey(eReadStream& src) {
        Key::readKey(src);
        mValue->read(src);
    }

    DrawableAutoTiledSurface& dSurface() { return *mValue.get(); }
private:
    const stdsptr<DrawableAutoTiledSurface> mValue;
};

class CORE_EXPORT AnimatedSurface : public Animator {
    Q_OBJECT
    e_OBJECT
    //typedef InterpolationKeyT<AutoTiledSurface> ASKey;
protected:
    AnimatedSurface();

    QDomElement prp_writePropertyXEV_impl(const XevExporter& exp) const;
    void prp_readPropertyXEV_impl(const QDomElement& ele, const XevImporter& imp);
public:
    void prp_drawTimelineControls(
            QPainter * const p, const qreal pixelsPerFrame,
            const FrameRange &absFrameRange, const int rowHeight) {
        const int width = qCeil(absFrameRange.span()*pixelsPerFrame);
        const QRect drawRect(0, 0, width, rowHeight);
        mFrameImagesCache.drawCacheOnTimeline(p, drawRect, absFrameRange.fMin,
                                              absFrameRange.fMax);
        Animator::prp_drawTimelineControls(p, pixelsPerFrame,
                                       absFrameRange,
                                       rowHeight);
    }

    FrameRange prp_getIdenticalRelRange(const int relFrame) const {
        const auto at = anim_getKeyAtRelFrame(relFrame);
        const auto neighs = anim_getPrevAndNextKey(relFrame);
        const auto prev = at ? at : neighs.first;
        const auto next = neighs.second;
        const int min = prev ? prev->getRelFrame() : FrameRange::EMIN;
        const int max = next ? next->getRelFrame() - 1 : FrameRange::EMAX;
        return {min, max};
    }

    void clearInUse() {
        mUsed.clear();
        mUseRange = {1, 0};
    }

    void clearUseRange() { setUseRange({1, 0}); }

    void setUseRange(const FrameRange& relRange) {
        mUsed.clear();
        mUseRange = relRange;
        if(mUseRange.isValid()) {
            const auto minId = anim_getNextKeyId(relRange.fMin - 1);
            const auto maxId = anim_getPrevKeyId(relRange.fMax + 1);
            if(minId == -1 || maxId == -1) return;
            for(int i = minId; i <= maxId; i++) {
                const auto iKey = anim_getKeyAtIndex<ASKey>(i);
                auto& dSurf = iKey->dSurface();
                mUsed.append(&dSurf);
                if(!dSurf.storesDataInMemory())
                    dSurf.scheduleLoadFromTmpFile();
            }
        }
    }

    void setupOnionSkinFor(const int sideRange, OnionSkin &skins,
                           const std::function<void()>& missingLoaded) {
        setupOnionSkinFor(anim_getCurrentRelFrame(),
                          sideRange, skins, missingLoaded);
    }

    void setupOnionSkinFor(const int relFrame, const int sideRange,
                           ASKey* key, OnionSkin::SkinsSide &side,
                           const std::function<void()>& missingLoaded) {
        const int dFrame = qAbs(relFrame - key->getRelFrame());
        if(dFrame > sideRange) return;
        const auto surf = &key->dSurface();
        if(!surf->storesDataInMemory()) {
            const auto task = surf->scheduleLoadFromTmpFile();
            if(task) task->addDependent({missingLoaded, nullptr});
            return;
        }
        const qreal weight = 1.*(sideRange - dFrame)/sideRange;
        side.fSkins.append({surf, toSkScalar(weight)});
    }

    void setupOnionSkinFor(const int relFrame, const int sideRange, OnionSkin &skins,
                           const std::function<void()>& missingLoaded);

    stdsptr<Key> anim_createKey() {
        return enve::make_shared<ASKey>(this);
    }

    void prp_readProperty_impl(eReadStream& src);
    void prp_writeProperty_impl(eWriteStream& dst) const;

    void prp_afterChangedAbsRange(const FrameRange &range, const bool clip);

    void anim_addKeyAtRelFrame(const int relFrame);

    void anim_setAbsFrame(const int frame) {
        Animator::anim_setAbsFrame(frame);
        updateCurrent();
    }

    DrawableAutoTiledSurface * getSurface(const int relFrame);

    void afterSurfaceChanged(DrawableAutoTiledSurface* const surface);

    DrawableAutoTiledSurface * getCurrentSurface() {
        return mCurrent_d;
    }

    void newEmptyFrame() { newEmptyFrame(anim_getCurrentRelFrame()); }
    void newEmptyFrame(const int relFrame);

    void loadPixmap(const QImage& src);
    void loadPixmap(const sk_sp<SkImage>& src);
    void loadPixmap(const SkPixmap& src);

    //! If the image is available returns nullptr,
    //! otherwise returns a task that has to finish for the image to be available,
    //! the task may also may be nullptr if something goes wrong
    eTask* getFrameImage(const int relFrame, sk_sp<SkImage>& img);

    void afterChangedCurrentContent();
    void addUndoRedo(const QString &name, const QRect &roi);

    eTaskBase* savePaintSVG(SvgExporter& exp, QDomElement& use,
                            const FrameRange& visRelRange);
signals:
    void currentSurfaceChanged(DrawableAutoTiledSurface*);
private:
    template <typename T>
    void loadPixmapT(const T &src);

    void updateCurrent();
    void setCurrent(DrawableAutoTiledSurface * const surf);

    HddCachableCacheHandler mFrameImagesCache;
    QList<UsePointer<DrawableAutoTiledSurface>> mUsed;
    FrameRange mUseRange{1, 0};
    const stdsptr<DrawableAutoTiledSurface> mBaseValue;
    DrawableAutoTiledSurface * mCurrent_d = nullptr;
};

#endif // ANIMATEDSURFACE_H
