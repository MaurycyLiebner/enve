#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "Animators/graphanimator.h"
#include "Animators/interpolationkeyt.h"
#include "drawableautotiledsurface.h"
#include "Animators/qrealpoint.h"
class AnimatedSurface;
class ASKey : public GraphKey {
    friend class StdSelfRef;
protected:
    ASKey(const int& frame, AnimatedSurface * const parent);
    ASKey(const DrawableAutoTiledSurface &value,
          const int& frame, AnimatedSurface * const parent);
public:
    bool differsFromKey(Key * const key) const {
        return key != this;
    }

    qreal getValueForGraph() const {
        return this->mRelFrame;
    }

    void setValueForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    void setRelFrame(const int &frame) {
        if(frame == this->mRelFrame) return;
        const int dFrame = frame - this->mRelFrame;
        GraphKey::setRelFrame(frame);
        this->mEndValue += dFrame;
        this->mStartValue += dFrame;
    }

    DrawableAutoTiledSurface& dSurface() { return mValue; }
private:
    DrawableAutoTiledSurface mValue;
};

class AnimatedSurface : public GraphAnimator {
    Q_OBJECT
    friend class SelfRef;
    //typedef InterpolationKeyT<AutoTiledSurface> ASKey;
protected:
    AnimatedSurface();
public:
    enum Interpolation {
        INTER_FADE,
        INTER_STEP
    };

    struct Interpolator {
        Interpolation fInter;
        SkIPoint fPos1;
        SkBitmap fBtmp1;
        SkIPoint fPos2;
        SkBitmap fBtmp2;
        SkScalar fWeight2;

        void draw(SkCanvas * const canvas) {
            if(fBtmp2.isNull() || fInter == INTER_STEP) {
                if(fBtmp1.isNull()) return;
                canvas->drawBitmap(fBtmp1, fPos1.x(), fPos1.y());
                return;
            }
            if(fBtmp1.isNull()) {
                canvas->drawBitmap(fBtmp2, fPos2.x(), fPos2.y());
                return;
            }
            const auto l = qMin(fPos1.x(), fPos2.x());
            const auto t = qMin(fPos1.y(), fPos2.y());
            const auto r = qMax(fPos1.x() + fBtmp1.width(),
                                fPos2.x() + fBtmp2.width());
            const auto b = qMax(fPos1.y() + fBtmp1.height(),
                                fPos2.y() + fBtmp2.height());
            const SkRect rHint = SkRect::MakeLTRB(l, t, r, b);
            canvas->saveLayer(rHint, nullptr);
            canvas->drawBitmap(fBtmp1, fPos1.x(), fPos1.y());
            SkPaint paint;
            if(fInter == INTER_FADE) {
                const int alpha =  clamp(qRound(fWeight2*255), 0, 255);
                paint.setAlpha(static_cast<U8CPU>(alpha));
            }
            canvas->drawBitmap(fBtmp2, fPos2.x(), fPos2.y(), &paint);
            canvas->restore();
        }
    };

    void anim_saveCurrentValueAsKey() {
        anim_addKeyAtRelFrame(anim_getCurrentRelFrame());
    }

    void anim_addKeyAtRelFrame(const int& relFrame) {
        if(anim_getKeyAtRelFrame(relFrame)) return;
        const auto prevNextKey = anim_getPrevAndNextKey<ASKey>(relFrame);
        stdsptr<ASKey> newKey;
        if(prevNextKey.first) {
            const auto& value = prevNextKey.first->dSurface();
            newKey = SPtrCreate(ASKey)(value, relFrame, this);
        } else if(prevNextKey.second) {
            const auto& value = prevNextKey.second->dSurface();
            newKey = SPtrCreate(ASKey)(value, relFrame, this);
        } else {
            newKey = SPtrCreate(ASKey)(mBaseValue, relFrame, this);
        }
        anim_appendKey(newKey);
    }

    void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        const auto spk = static_cast<ASKey*>(key);
        if(spk) setCurrent(&spk->dSurface());
        else {
            const auto relFrame = anim_getCurrentRelFrame();
            const auto prevNextKey = anim_getPrevAndNextKey<ASKey>(relFrame);
            if(prevNextKey.first) {
                setCurrent(&prevNextKey.first->dSurface());
            } else if(prevNextKey.second) {
                setCurrent(&prevNextKey.second->dSurface());
            } else {
                setCurrent(&mBaseValue);
            }
        }
    }

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType &type,
            qreal &minValue, qreal &maxValue) const {
        if(type == QrealPointType::KEY_POINT) {
            minValue = key->getRelFrame();
            maxValue = minValue;
            //getFrameConstraints(key, type, minValue, maxValue);
        } else {
            minValue = -DBL_MAX;
            maxValue = DBL_MAX;
        }
    }

    DrawableAutoTiledSurface * getCurrentSurface() {
        return mCurrent_d;
    }
signals:
    void currentSurfaceChanged(DrawableAutoTiledSurface*);
private:
    void setCurrent(DrawableAutoTiledSurface * const surf) {
        if(mCurrent_d == surf) return;
        mCurrent_d = surf;
        emit currentSurfaceChanged(mCurrent_d);
    }

    DrawableAutoTiledSurface mBaseValue;
    DrawableAutoTiledSurface * mCurrent_d = &mBaseValue;
};

#endif // ANIMATEDSURFACE_H
