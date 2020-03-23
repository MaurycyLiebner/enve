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

#include "Tasks/domeletask.h"

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

class ASurfaceSaverSVG : public ComplexTask {
public:
    ASurfaceSaverSVG(AnimatedSurface* const src,
                     SvgExporter& exp, QDomElement& parent,
                     const qreal div, const FrameRange& relRange,
                     const FrameRange& visRelRange) :
        ComplexTask(relRange.fMax, "SVG Paint Object"),
        mSrc(src), mExp(exp), mParent(parent),
        mRelRange(relRange), mVisRage(visRelRange),
        mDiv(div), mKeyRelFrame(relRange.fMin - 1) {}

    void nextStep() override {
        if(!mSrc) return cancel();
        if(setValue(mKeyRelFrame)) return finish();
        if(done()) return;

        bool first = mKeyId == 0;
        if(first) {
            const auto idRange = mSrc->prp_getIdenticalRelRange(mVisRage.fMin);
            const int span = mExp.fAbsRange.span();

            if(idRange.inRange(mVisRage) || span == 1) {
                addSurface(mVisRage.fMin, nullptr);
                mKeyRelFrame = mVisRage.fMax;
                return nextStep();
            }
        }

        const auto& keys = mSrc->anim_getKeys();
        if(mKeyId >= keys.count()) {
            mKeyRelFrame = mVisRage.fMax;
            return nextStep();
        }
        const auto key = static_cast<ASKey*>(keys.atId(mKeyId++));
        mKeyRelFrame = key->getRelFrame();
        if(mKeyRelFrame >= mVisRage.fMax) return nextStep();
        if(mKeyRelFrame >= mVisRage.fMin) {
            DrawableAutoTiledSurface* surf = nullptr;
            if(first) {
                first = false;
                const int prevId = mKeyId - 2;
                const auto null = static_cast<Key*>(nullptr);
                const auto prevKey = prevId >= 0 ? keys.atId(prevId) : null;
                const auto prevASKey = static_cast<ASKey*>(prevKey);
                const bool keyOnFirstFrame = mKeyRelFrame == mVisRage.fMin;
                const bool useKey = keyOnFirstFrame || !prevKey;
                const auto firstKey = useKey ? key : prevASKey;
                surf = &firstKey->dSurface();
            } else surf = &key->dSurface();
            const bool wait = addSurface(mKeyRelFrame, surf);
            if(!wait) addEmptyTask();
        } else nextStep();
    }

private:
    //! @brief Returns true if there is a task, does have to wait.
    bool addSurface(const int relFrame, DrawableAutoTiledSurface* surf) {
        if(!surf) surf = mSrc->getSurface(relFrame);
        const QString imageId = SvgExportHelpers::ptrToStr(surf);
        const QPoint pos = -surf->zeroTilePos();
        sk_sp<SkImage> image;
        const auto task = mSrc->getFrameImage(relFrame, image);
        if(task) {
            const auto imgTask = static_cast<ImgLoader*>(task);
            const QPointer<ASurfaceSaverSVG> ptr = this;
            task->addDependent({[ptr, imgTask, relFrame, imageId, pos]() {
                if(!ptr) return;
                const auto image = imgTask->image();
                ptr->saveSurfaceValues(relFrame, image, imageId, pos);
            }, nullptr});
            addTask(task->ref<eTask>());
            return true;
        } else {
            saveSurfaceValues(relFrame, image, imageId, pos);
            return false;
        }
    }

    void saveSurfaceValues(const int relFrame, const sk_sp<SkImage>& image,
                           const QString& imageId, const QPoint& pos) {
        const qreal t = (relFrame - mRelRange.fMin)/mDiv;
        mKeyTimes << QString::number(t);

        SvgExportHelpers::defImage(mExp, image, imageId);
        mHrefValues << "#" + imageId;

        mXValues << QString::number(pos.x());
        mYValues << QString::number(pos.y());
    }

    void finish() {
        if(mHrefValues.isEmpty()) return;

        auto use = mExp.createElement("use");

        if(mHrefValues.count() == 1) {
            const QString href = mHrefValues.first();
            const QString x = mXValues.first();
            const QString y = mYValues.first();
            use.setAttribute("href", href);
            use.setAttribute("x", x);
            use.setAttribute("y", y);
        } else {
            if(mKeyTimes.last() != "1") {
                mHrefValues << mHrefValues.last();
                mXValues << mXValues.last();
                mYValues << mYValues.last();
                mKeyTimes << "1";
            }

            const qreal dur = mDiv/mExp.fFps;
            const auto durStr = QString::number(dur)  + 's';
            const auto keyTimesStr = mKeyTimes.join(';');
            {
                auto anim = mExp.createElement("animate");
                anim.setAttribute("attributeName", "href");
                anim.setAttribute("dur", durStr);
                anim.setAttribute("values", mHrefValues.join(';'));
                anim.setAttribute("keyTimes", keyTimesStr);
                SvgExportHelpers::assignLoop(anim, mExp.fLoop);
                use.appendChild(anim);
            }
            {
                auto anim = mExp.createElement("animate");
                anim.setAttribute("attributeName", "x");
                anim.setAttribute("dur", durStr);
                anim.setAttribute("values", mXValues.join(';'));
                anim.setAttribute("keyTimes", keyTimesStr);
                SvgExportHelpers::assignLoop(anim, mExp.fLoop);
                use.appendChild(anim);
            }
            {
                auto anim = mExp.createElement("animate");
                anim.setAttribute("attributeName", "y");
                anim.setAttribute("dur", durStr);
                anim.setAttribute("values", mYValues.join(';'));
                anim.setAttribute("keyTimes", keyTimesStr);
                SvgExportHelpers::assignLoop(anim, mExp.fLoop);
                use.appendChild(anim);
            }
        }

        mParent.appendChild(use);
    }

    const QPointer<AnimatedSurface> mSrc;
    SvgExporter& mExp;
    QDomElement& mParent;
    const FrameRange mRelRange;
    const FrameRange mVisRage;
    const qreal mDiv;

    int mKeyRelFrame = 0;
    int mKeyId = 0;

    QStringList mHrefValues;
    QStringList mXValues;
    QStringList mYValues;
    QStringList mKeyTimes;
};

eTaskBase* AnimatedSurface::savePaintSVG(SvgExporter& exp, QDomElement& parent,
                                         const FrameRange& visRelRange) {
    const auto relRange = prp_absRangeToRelRange(exp.fAbsRange);
    const int span = exp.fAbsRange.span();
    const qreal div = span - 1;

    const auto task = new ASurfaceSaverSVG(this, exp, parent, div,
                                           relRange, visRelRange);
    const auto taskSPtr = QSharedPointer<ASurfaceSaverSVG>(
                              task, &QObject::deleteLater);
    task->nextStep();

    if(task->done()) return nullptr;
    TaskScheduler::instance()->addComplexTask(taskSPtr);
    return task;
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
