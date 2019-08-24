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

#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "Animators/graphanimator.h"
#include "Animators/interpolationkeyt.h"
#include "drawableautotiledsurface.h"
#include "Animators/qrealpoint.h"
class AnimatedSurface;
class ASKey : public Key {
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

class AnimatedSurface : public Animator {
    Q_OBJECT
    e_OBJECT
    //typedef InterpolationKeyT<AutoTiledSurface> ASKey;
protected:
    AnimatedSurface();
public:
    struct OnionSkin {
        struct Skin {
            DrawableAutoTiledSurface* fSurface;
            float fWeight;

            SkIRect boundingRect() const {
                return toSkIRect(fSurface->pixelBoundingRect());
            }
        };

        struct SkinsSide {
            SkColor4f fColor;
            QList<Skin> fSkins;
            sk_sp<SkImage> fImage;
            SkIPoint fImageXY;

            SkIRect boundingRect() const {
                SkIRect result = SkIRect::MakeXYWH(0, 0, 0, 0);
                bool first = true;
                for(const auto& skin : fSkins) {
                    if(first) {
                        result = skin.boundingRect();
                        first = false;
                    } else result.join(skin.boundingRect());
                }
                return result;
            }

            void draw(SkCanvas * const canvas) {
                if(fSkins.isEmpty()) return;
                if(!fImage) setupImage(canvas->getGrContext());
                SkPaint paint;
                paint.setAlphaf(0.5f);
                canvas->drawImage(fImage, fImageXY.x(), fImageXY.y(), &paint);
            }

            void clear() {
                fSkins.clear();
                fImage.reset();
            }

            void setupImage(GrContext* const grContext) {
                const auto bRect = boundingRect();
                if(bRect.width() <= 0 || bRect.height() <= 0) return;
                fImageXY = bRect.topLeft();
                const auto grTex = grContext->createBackendTexture(
                            bRect.width(), bRect.height(),
                            kRGBA_8888_SkColorType, GrMipMapped::kNo,
                            GrRenderable::kYes);

                sk_sp<SkSurface> gpuSurface = SkSurface::MakeFromBackendTexture(
                            grContext, grTex,
                            kTopLeft_GrSurfaceOrigin, 0,
                            kRGBA_8888_SkColorType,
                            nullptr, nullptr);
                const auto texCanvas = gpuSurface->getCanvas();
                texCanvas->clear(SK_ColorTRANSPARENT);
                texCanvas->translate(-fImageXY.x(), -fImageXY.y());
                for(const auto& skin : fSkins) {
                    SkPaint paint;
                    const float rgbMax = qMax(fColor.fR, qMax(fColor.fG, fColor.fB));
                    const float colM[20] = {
                        1 - rgbMax, 0, 0, fColor.fR, 0,
                        0, 1 - rgbMax, 0, fColor.fG, 0,
                        0, 0, 1 - rgbMax, fColor.fB, 0,
                        0, 0, 0, fColor.fA*skin.fWeight, 0};
                    const auto colF = SkColorFilters::Matrix(colM);
                    paint.setColorFilter(colF);

                    skin.fSurface->drawOnCanvas(texCanvas, {0, 0}, &paint);
                }
                texCanvas->flush();
                fImage = SkImage::MakeFromAdoptedTexture(grContext, grTex,
                                                         kTopLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType);
            }
        };

        SkinsSide fPrev{{1, 0, 0, 1}, QList<Skin>(), nullptr, {0, 0}};
        SkinsSide fNext{{0, 0.5f, 1, 1}, QList<Skin>(), nullptr, {0, 0}};

        void draw(SkCanvas * const canvas) {
            fPrev.draw(canvas);
            fNext.draw(canvas);
        }

        void clear() {
            fPrev.clear();
            fNext.clear();
        }
    };

    void setupOnionSkinFor(const int sideRange, OnionSkin &skins) {
        setupOnionSkinFor(anim_getCurrentRelFrame(), sideRange, skins);
    }

    void setupOnionSkinFor(const int relFrame, const int sideRange,
                           OnionSkin &skins) {
        skins.clear();
        ASKey * currKey = anim_getKeyAtRelFrame<ASKey>(relFrame);
        if(!currKey) currKey = anim_getPrevKey<ASKey>(relFrame);
        if(!currKey) currKey = anim_getNextKey<ASKey>(relFrame);
        if(!currKey) return;
        ASKey * prevKey = anim_getPrevKey<ASKey>(currKey);
        while(prevKey) {
            const auto surf = &prevKey->dSurface();
            const int dFrame = qAbs(relFrame - prevKey->getRelFrame());
            if(dFrame > sideRange) break;
            const qreal weight = 1.*(sideRange - dFrame)/sideRange;
            skins.fPrev.fSkins.append({surf, toSkScalar(weight)});
            prevKey = anim_getPrevKey<ASKey>(prevKey);
        }

        ASKey * nextKey = anim_getNextKey<ASKey>(currKey);
        while(nextKey) {
            const auto surf = &nextKey->dSurface();
            const int dFrame = qAbs(relFrame - nextKey->getRelFrame());
            if(dFrame > sideRange) break;
            const qreal weight = 1.*(sideRange - dFrame)/sideRange;
            skins.fNext.fSkins.append({surf, toSkScalar(weight)});
            nextKey = anim_getNextKey<ASKey>(nextKey);
        }
    }

    stdsptr<Key> createKey() {
        return enve::make_shared<ASKey>(this);
    }

    void readProperty(eReadStream& src) {
        Animator::readProperty(src);
        readKeys(src);
        mBaseValue->read(src);
    }

    void writeProperty(eWriteStream& dst) const {
        Animator::writeProperty(dst);
        writeKeys(dst);
        mBaseValue->write(dst);
    }

    void anim_saveCurrentValueAsKey() {
        anim_addKeyAtRelFrame(anim_getCurrentRelFrame());
    }

    void anim_addKeyAtRelFrame(const int relFrame) {
        if(anim_getKeyAtRelFrame(relFrame)) return;
        const auto prevNextKey = anim_getPrevAndNextKey<ASKey>(relFrame);
        stdsptr<ASKey> newKey;
        if(prevNextKey.first) {
            const auto& value = prevNextKey.first->dSurface();
            newKey = enve::make_shared<ASKey>(value, relFrame, this);
        } else if(prevNextKey.second) {
            const auto& value = prevNextKey.second->dSurface();
            newKey = enve::make_shared<ASKey>(value, relFrame, this);
        } else {
            newKey = enve::make_shared<ASKey>(*mBaseValue.get(), relFrame, this);
        }
        anim_appendKey(newKey);
    }

    void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        const auto spk = static_cast<ASKey*>(key);
        if(spk) setCurrent(&spk->dSurface());
        else {
            const int relFrame = anim_getCurrentRelFrame();
            const auto prevNextKey = anim_getPrevAndNextKey<ASKey>(relFrame);
            if(prevNextKey.first) {
                setCurrent(&prevNextKey.first->dSurface());
            } else if(prevNextKey.second) {
                setCurrent(&prevNextKey.second->dSurface());
            } else {
                setCurrent(mBaseValue.get());
            }
        }
    }

    DrawableAutoTiledSurface * getSurface(const int relFrame) {
        const auto spk = anim_getKeyAtRelFrame<ASKey>(relFrame);
        if(spk) return &spk->dSurface();
        const auto prevNextKey = anim_getPrevAndNextKey<ASKey>(relFrame);
        if(prevNextKey.first) {
            return &prevNextKey.first->dSurface();
        } else if(prevNextKey.second) {
            return &prevNextKey.second->dSurface();
        } else {
            return mBaseValue.get();
        }
    }

    DrawableAutoTiledSurface * getCurrentSurface() {
        return mCurrent_d;
    }

    void newEmptyFrame() {
        newEmptyFrame(anim_getCurrentRelFrame());
    }

    void newEmptyFrame(const int relFrame) {
        const auto currKey = anim_getKeyAtRelFrame<ASKey>(relFrame);
        if(currKey) anim_removeKey(currKey->ref<ASKey>());
        const auto newKey = enve::make_shared<ASKey>(relFrame, this);
        anim_appendKey(newKey);
    }
signals:
    void currentSurfaceChanged(DrawableAutoTiledSurface*);
private:
    void setCurrent(DrawableAutoTiledSurface * const surf) {
        if(mCurrent_d == surf) return;
        mCurrent_d = surf;
        emit currentSurfaceChanged(mCurrent_d);
    }

    const stdsptr<DrawableAutoTiledSurface> mBaseValue;
    DrawableAutoTiledSurface * mCurrent_d = nullptr;
};

#endif // ANIMATEDSURFACE_H
