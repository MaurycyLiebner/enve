// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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
