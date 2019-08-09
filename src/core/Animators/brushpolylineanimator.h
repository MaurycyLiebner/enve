#ifndef BRUSHPOLYLINEANIMATOR_H
#define BRUSHPOLYLINEANIMATOR_H
#include "brushpolyline.h"
#include "graphanimator.h"
#include "interpolationkeyt.h"
#include "transformanimator.h"

class BrushPolylineAnimator : public GraphAnimator {
    e_OBJECT
    typedef InterpolationKeyT<BrushPolyline> BKey;
protected:
    BrushPolylineAnimator();
public:
    void drawCanvasControls(SkCanvas * const canvas,
                            const CanvasMode mode,
                            const float invScale) {
        if(mode != CanvasMode::pointTransform) return;
        const auto trans = getTransformAnimator();
        if(!trans) return;
        const auto begin = mPathBeingChanged_d->begin();
        const auto end = mPathBeingChanged_d->end();
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setStyle(SkPaint::kFill_Style);
        SkPaint linePaint;
        linePaint.setColor(SK_ColorBLUE);
        linePaint.setStyle(SkPaint::kStroke_Style);
        linePaint.setStrokeWidth(invScale);
        const auto& firstPt = reinterpret_cast<const QPointF&>(*begin);
        SkPoint lastPos = toSkPoint(firstPt);
        for(auto it = begin; it != end; it++) {
            const QPointF& pos = reinterpret_cast<const QPointF&>(*it);
            const auto absPos = trans->mapRelPosToAbs(pos);
            const auto absPosSk = toSkPoint(absPos);
            canvas->drawLine(lastPos, absPosSk, linePaint);
            canvas->drawCircle(absPosSk, 3, paint);
            lastPos = absPosSk;
        }
    }

    void anim_saveCurrentValueAsKey() {
        if(anim_getKeyOnCurrentFrame()) return;
        const auto newKey = enve::make_shared<BKey>(
                    mBaseValue, anim_getCurrentRelFrame(), this);
        anim_appendKey(newKey);
    }

    void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        const auto spk = static_cast<BKey*>(key);
        if(spk) mPathBeingChanged_d = &spk->getValue();
        else mPathBeingChanged_d = &mBaseValue;
    }

    void beforeBinaryPathChange() {
        if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
            anim_saveCurrentValueAsKey();
        }
    }

    void startPathChange() {
        if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
            anim_saveCurrentValueAsKey();
        }
        mPathBeingChanged_d->save();
    }

    void pathChanged() {
        const auto spk = anim_getKeyOnCurrentFrame<BKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_afterWholeInfluenceRangeChanged();
        }
    }

    void cancelPathChange() {
        mPathBeingChanged_d->restore();
        const auto spk = anim_getKeyOnCurrentFrame<BKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_afterWholeInfluenceRangeChanged();
        }
    }

    void finishPathChange() {
        const auto spk = anim_getKeyOnCurrentFrame<BKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_afterWholeInfluenceRangeChanged();
        }
    }

private:
    BrushPolyline mBaseValue;
    BrushPolyline * mPathBeingChanged_d = &mBaseValue;
};

#endif // BRUSHPOLYLINEANIMATOR_H
