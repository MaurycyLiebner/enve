#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimator.h"
#include "smartpathcontainer.h"
#include "Animators/interpolationanimatort.h"
#include "differsinterpolate.h"
#include "basicreadwrite.h"
class PathAnimator;
class SmartPathKey : public InterpolationKeyT<SmartPath> {
    friend class StdSelfRef;
public:
    void save() {
        mValue.save();
    }

    void restore() {
        mValue.restore();
    }

    void updateAfterPrevKeyChanged(Key * const prevKey) {
        mValue.setPrev(&static_cast<SmartPathKey*>(prevKey)->getValue());
    }

    void updateAfterNextKeyChanged(Key * const nextKey) {
        mValue.setNext(&static_cast<SmartPathKey*>(nextKey)->getValue());
    }
protected:
    SmartPathKey(const SmartPath& value, const int &relFrame,
                 Animator * const parentAnimator) :
        InterpolationKeyT<SmartPath>(value, relFrame, parentAnimator) {}
    SmartPathKey(Animator * const parentAnimator) :
        InterpolationKeyT<SmartPath>(parentAnimator) {}
};

class SmartPathAnimator : public GraphAnimator {
    friend class SelfRef;
public:
    bool SWT_isSmartPathAnimator() const { return true; }

    SkPath getPathAtAbsFrame(const qreal &frame) const {
        return getPathAtRelFrame(this->prp_absFrameToRelFrameF(frame));
    }

    qreal getInterpolatedFrameAtRelFrame(
            const qreal &frame) const {
        int prevId;
        int nextId;
        if(this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, frame)) {
            if(nextId == prevId) {
                return GetAsGK(this->anim_mKeys.at(nextId))->getRelFrame();
            } else {
                GraphKey *prevKey = GetAsGK(this->anim_mKeys.at(prevId));
                GraphKey *nextKey = GetAsGK(this->anim_mKeys.at(nextId));

                qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                     prevKey->getEndFrame(),
                                     nextKey->getStartFrame(),
                                     qreal(nextKey->getRelFrame())};
                qreal t = gTFromX(seg, frame);
                qreal p0y = prevKey->getValueForGraph();
                qreal p1y = prevKey->getEndValue();
                qreal p2y = nextKey->getStartValue();
                qreal p3y = nextKey->getValueForGraph();
                return gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
            }
        }
        return frame;
    }

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType &type,
            qreal &minValue, qreal &maxValue) const;


    SkPath getPathAtRelFrame(const qreal &frame) const {
        if(this->anim_mKeys.isEmpty()) return mBaseValue.getPathAt();
        int prevId;
        int nextId;
        if(this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId,
                                                          frame)) {
            if(nextId == prevId) {
                return getKeyAtId(nextId)->getValue().getPathAt();
            } else {
                const auto * const prevKey = getKeyAtId(prevId);
                const auto * const nextKey = getKeyAtId(nextId);

                const qreal prevFrame = prevKey->getRelFrame();
                const qreal nextFrame = nextKey->getRelFrame();

                const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                          prevKey->getEndFrame(),
                                          nextKey->getStartFrame(),
                                          qreal(nextKey->getRelFrame())};
                const qreal t = gTFromX(seg, frame);
                const qreal p0y = prevKey->getValueForGraph();
                const qreal p1y = prevKey->getEndValue();
                const qreal p2y = nextKey->getStartValue();
                const qreal p3y = nextKey->getValueForGraph();
                const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
                const qreal dFrame = nextFrame - prevFrame;
                const qreal nWeight = (iFrame - prevFrame)/dFrame;
                return prevKey->getValue().interpolateWithNext(nWeight);
            }
        }
        return mBaseValue.getPathAt();
    }

    SmartPath * getCurrentlyEditedPath() const {
        return mPathBeingChanged_d;
    }

    void currentlyEditedPathChanged() {
        if(anim_mKeyOnCurrentFrame) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void startPathChange() {
        if(mPathBeingChanged_d) return;
        if(anim_isRecording() && !anim_mKeyOnCurrentFrame) {
            anim_saveCurrentValueAsKey();
        }
        if(anim_mKeyOnCurrentFrame) {
            const auto spk = GetAsPtr(anim_mKeyOnCurrentFrame,
                                      SmartPathKey);
            mPathBeingChanged_d = &spk->getValue();
        } else {
            mPathBeingChanged_d = &mBaseValue;
        }
        mPathBeingChanged_d->save();
    }

    void cancelPathChange() {
        if(!mPathBeingChanged_d) return;
        mPathBeingChanged_d->restore();
        if(anim_mKeyOnCurrentFrame) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathBeingChanged_d = nullptr;
    }

    void finishPathChange() {
        if(!mPathBeingChanged_d) return;
        if(anim_isKeyOnCurrentFrame()) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
        mPathBeingChanged_d = nullptr;
    }
protected:
    SmartPathAnimator(PathAnimator * const pathAnimator);
private:
    SmartPath * mPathBeingChanged_d = nullptr;
    SmartPath mBaseValue;

    SmartPathKey* getKeyAtId(const int& id) const {
        return GetAsPtrTemplated(this->anim_mKeys.at(id), SmartPathKey);
    }
};

#endif // SMARTPATHANIMATOR_H
