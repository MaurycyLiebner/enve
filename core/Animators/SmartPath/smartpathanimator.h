#ifndef SMARTPATHANIMATOR_H
#define SMARTPATHANIMATOR_H
#include "Animators/interpolationanimatort.h"
#include "differsinterpolate.h"
#include "basicreadwrite.h"
#include "smartpathkey.h"

class SmartPathAnimator : public GraphAnimator {
    friend class SelfRef;
    Q_OBJECT
public:
    bool SWT_isSmartPathAnimator() const { return true; }

    void anim_setAbsFrame(const int &frame) {
        if(frame == anim_getCurrentAbsFrame()) return;
        const int lastRelFrame = anim_getCurrentRelFrame();
        Animator::anim_setAbsFrame(frame);
        if(anim_hasKeys()) {
            const auto prevK1 = anim_getPrevKey<SmartPathKey>(lastRelFrame);
            const auto prevK2 = anim_getPrevKey<SmartPathKey>(anim_getCurrentRelFrame());
            const auto nextK1 = anim_getNextKey<SmartPathKey>(lastRelFrame);
            const auto nextK2 = anim_getNextKey<SmartPathKey>(anim_getCurrentRelFrame());
            const auto keyAtFrame1 = anim_getKeyAtRelFrame(lastRelFrame);
            const auto keyAtFrame2 = anim_getKeyOnCurrentFrame<SmartPathKey>();
            if(!prevK1 && !prevK2 && !keyAtFrame1 && !keyAtFrame2) return;
            if(!nextK1 && !nextK2 && !keyAtFrame1 && !keyAtFrame2) return;
            if(keyAtFrame2) {
                mBaseValue.assign(keyAtFrame2->getValue());
            } else if(prevK2 && nextK2) {
                const qreal nWeight =
                        prevKeyWeight(prevK2, nextK2, anim_getCurrentRelFrame());
                gInterpolate(prevK2->getValue(), nextK2->getValue(),
                             nWeight, mBaseValue);
            } else if(prevK2) {
                mBaseValue.assign(prevK2->getValue());
            } else if(nextK2) {
                mBaseValue.assign(nextK2->getValue());
            }
            anim_callFrameChangeUpdater();
        }
    }

    SkPath getPathAtAbsFrame(const qreal &frame) const {
        return getPathAtRelFrame(prp_absFrameToRelFrameF(frame));
    }

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType &type,
            qreal &minValue, qreal &maxValue) const;

    qreal prevKeyWeight(const SmartPathKey * const prevKey,
                        const SmartPathKey * const nextKey,
                        const qreal & frame) const {
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
        const qreal pWeight = (iFrame - prevFrame)/dFrame;
        return pWeight;
    }

    SkPath getPathAtRelFrame(const qreal &frame) const {
        if(anim_mKeys.isEmpty()) return mBaseValue.getPathAt();
        const auto pn = anim_getPrevAndNextKeyIdForRelFrameF(frame);
        const int prevId = pn.first;
        const int nextId = pn.second;

        const auto prevKey = anim_getKeyAtIndex<SmartPathKey>(prevId);
        const auto nextKey = anim_getKeyAtIndex<SmartPathKey>(nextId);
        const bool adjKeys = pn.second - pn.first == 1;
        const auto keyAtRelFrame = adjKeys ?
                    nullptr :
                    anim_getKeyAtIndex<SmartPathKey>(pn.first + 1);
        if(keyAtRelFrame) return keyAtRelFrame->getValue().getPathAt();
        if(prevKey && nextKey) {
            SkPath result;
            const qreal pWeight = prevKeyWeight(prevKey, nextKey, frame);
            nextKey->getPath().interpolate(prevKey->getPath(),
                                           toSkScalar(pWeight),
                                           &result);
            return result;
        } else if(!prevKey && nextKey) {
            return nextKey->getValue().getPathAt();
        } else if(prevKey && !nextKey) {
            return prevKey->getValue().getPathAt();
        }
        return mBaseValue.getPathAt();
    }

    SmartPath * getCurrentlyEditedPath() const {
        return mPathBeingChanged_d;
    }

    void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        const auto spk = static_cast<SmartPathKey*>(key);
        if(spk) {
            mPathBeingChanged_d = &spk->getValue();
        } else {
            mPathBeingChanged_d = &mBaseValue;
        }
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

    bool isClosed() const {
        return mBaseValue.isClosed();
    }

    void pathChanged() {
        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void cancelPathChange() {
        mPathBeingChanged_d->restore();
        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void finishPathChange() {
        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            anim_updateAfterChangedKey(spk);
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    void anim_saveCurrentValueAsKey() {
        if(!anim_isRecording()) anim_setRecording(true);

        const auto spk = anim_getKeyOnCurrentFrame<SmartPathKey>();
        if(spk) {
            spk->assignValue(mBaseValue);
        } else {
            const auto newKey = SPtrCreate(SmartPathKey)(
                        mBaseValue, anim_getCurrentRelFrame(), this);
            anim_appendKey(newKey);
        }
    }

    void writeProperty(QIODevice * const target) const {
        const int nKeys = anim_mKeys.count();
        target->write(rcConstChar(&nKeys), sizeof(int));
        for(const auto &key : anim_mKeys) {
            key->writeKey(target);
        }
        gWrite(target, mBaseValue);
    }

    void readProperty(QIODevice *target) {
        int nKeys;
        target->read(rcChar(&nKeys), sizeof(int));
        for(int i = 0; i < nKeys; i++) {
            auto newKey = SPtrCreate(SmartPathKey)(this);
            newKey->readKey(target);
            anim_appendKey(newKey);
        }
        gRead(target, mBaseValue);
    }

    void actionRemoveNode(const int& nodeId) {
        for(const auto &key : anim_mKeys) {
            const auto spKey = GetAsPtr(key, SmartPathKey);
            spKey->getValue().actionRemoveNode(nodeId);
        }
        mBaseValue.actionRemoveNode(nodeId);
        prp_updateInfluenceRangeAfterChanged();
    }

    int actionAddNewAtEnd(const int& nodeId,
                          const QPointF &relPos) {
        beforeBinaryPathChange();
        for(const auto &key : anim_mKeys) {
            const auto spKey = GetAsPtr(key, SmartPathKey);
            spKey->getValue().actionAppendNodeAtEndNode(nodeId);
        }
        const int id = mBaseValue.actionAppendNodeAtEndNode(nodeId);
        getCurrentlyEditedPath()->actionSetNormalNodeValues(
                    id, relPos, relPos, relPos);
        prp_updateInfluenceRangeAfterChanged();
        return id;
    }

    int actionAddFirstNode(const QPointF &relPos) {
        for(const auto &key : anim_mKeys) {
            const auto spKey = GetAsPtr(key, SmartPathKey);
            spKey->getValue().actionAddFirstNode(relPos, relPos, relPos);
        }
        const int id = mBaseValue.actionAddFirstNode(relPos, relPos, relPos);
        prp_updateInfluenceRangeAfterChanged();
        return id;
    }

    int actionInsertNodeBetween(const int &node1Id,
                                const int &node2Id,
                                const qreal &t) {
        beforeBinaryPathChange();
        for(const auto &key : anim_mKeys) {
            const auto spKey = GetAsPtr(key, SmartPathKey);
            spKey->getValue().actionInsertNodeBetween(node1Id, node2Id, t);
        }
        const int id = mBaseValue.actionInsertNodeBetween(node1Id, node2Id, t);
        getCurrentlyEditedPath()->actionPromoteDissolvedNodeToNormal(id);
        prp_updateInfluenceRangeAfterChanged();
        return id;
    }

    void actionConnectNodes(const int &node1Id, const int &node2Id) {
        for(const auto &key : anim_mKeys) {
            const auto spKey = GetAsPtr(key, SmartPathKey);
            spKey->getValue().actionConnectNodes(node1Id, node2Id);
        }
        mBaseValue.actionConnectNodes(node1Id, node2Id);
        prp_updateInfluenceRangeAfterChanged();
    }

    void actionDisconnectNodes(const int &node1Id, const int &node2Id) {
        for(const auto &key : anim_mKeys) {
            const auto spKey = GetAsPtr(key, SmartPathKey);
            spKey->getValue().actionDisconnectNodes(node1Id, node2Id);
        }
        mBaseValue.actionDisconnectNodes(node1Id, node2Id);
        prp_updateInfluenceRangeAfterChanged();
    }
signals:
    void pathChangedAfterFrameChange();
protected:
    SmartPathAnimator();
private:
    SmartPath mBaseValue;
    SmartPath * mPathBeingChanged_d = &mBaseValue;
};

#endif // SMARTPATHANIMATOR_H
