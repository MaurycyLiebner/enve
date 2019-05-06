#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include "Animators/graphanimator.h"
class QrealKey;
class QrealPoint;
class RandomQrealGenerator;

class QrealAnimator :  public GraphAnimator {
    Q_OBJECT
    friend class SelfRef;
protected:
    QrealAnimator(const QString& name);
    QrealAnimator(const qreal &iniVal,
                  const qreal &minVal,
                  const qreal &maxVal,
                  const qreal &prefferdStep,
                  const QString& name);
public:
    class Snapshot {
        struct KeySnaphot {
            qreal fC0Frame;
            qreal fC0Value;

            int fFrame;
            qreal fValue;

            qreal fC1Frame;
            qreal fC1Value;
        };
    public:
        Snapshot() {}
        Snapshot(const qreal& currentValue) : mCurrentValue(currentValue) {}

        void appendKey(const QrealKey * const key);

        qreal getValue(const qreal& relFrame) const {
            const KeySnaphot * prevKey = nullptr;
            const KeySnaphot * nextKey = nullptr;

            getPrevAndNextKey(relFrame, prevKey, nextKey);

            if(prevKey && nextKey) {
                const qCubicSegment2D seg{{qreal(prevKey->fFrame), prevKey->fValue},
                                          {prevKey->fC1Frame, prevKey->fC1Value},
                                          {nextKey->fC0Frame, nextKey->fC0Value},
                                          {qreal(nextKey->fFrame), nextKey->fValue}};
                auto frameSeg =  seg.xSeg();
                const qreal t = frameSeg.tAtLength(relFrame - frameSeg.p0());
                const auto valSeg = seg.ySeg();
                return valSeg.valAtT(t);
            } else if(prevKey) {
                return prevKey->fValue;
            } else if(nextKey) {
                return nextKey->fValue;
            }
            return mCurrentValue;
        }
    private:
        void getPrevAndNextKey(const qreal& relFrame,
                               KeySnaphot const *& prevKey,
                               KeySnaphot const *& nextKey) const {
            getPrevAndNextKey(relFrame, prevKey, nextKey, 0, mKeys.count() - 1);
        }

        void getPrevAndNextKey(const qreal& relFrame,
                               KeySnaphot const *& prevKey,
                               KeySnaphot const *& nextKey,
                               const int& minId, const int& maxId) const {
            if(maxId < minId) return;
            if(maxId - minId == 0) {
                prevKey = &mKeys[minId];
                return;
            } else if(maxId - minId == 1) {
                const auto minKey = &mKeys[minId];
                const auto maxKey = &mKeys[maxId];
                prevKey = maxKey->fFrame > relFrame ? minKey : nullptr;
                nextKey = minKey->fFrame < relFrame ? maxKey : nullptr;
                return;
            }
            const int guessId = (minId + maxId)/2;
            const auto& guessKey = &mKeys[guessId];
            if(guessKey->fFrame > relFrame) {
                getPrevAndNextKey(relFrame, prevKey, nextKey, minId, guessId);
                return;
            } else if(guessKey->fFrame > relFrame) {
                getPrevAndNextKey(relFrame, prevKey, nextKey, guessId, maxId);
                return;
            }
            prevKey = guessKey;
            return;
        }

        qreal mCurrentValue;
        QList<KeySnaphot> mKeys;
    };

    bool SWT_isQrealAnimator() const { return true; }
    void prp_startTransform();
    void prp_finishTransform();
    void prp_cancelTransform();
    QString prp_getValueText();
    void prp_setTransformed(const bool &bT) { mTransformed = bT; }
    void prp_afterChangedAbsRange(const FrameRange& range) {
        if(range.inRange(anim_getCurrentAbsFrame()))
            updateBaseValueFromCurrentFrame();
        GraphAnimator::prp_afterChangedAbsRange(range);
    }

    void anim_setAbsFrame(const int &frame);
    void anim_removeAllKeys();
    void anim_addKeyAtRelFrame(const int& relFrame);

    QPainterPath graph_getPathForSegment(
            const GraphKey * const prevKey,
            const GraphKey * const nextKey) const;

    qValueRange graph_getMinAndMaxValues() const;
    qValueRange graph_getMinAndMaxValuesBetweenFrames(
            const int &startFrame, const int &endFrame) const;

    qreal graph_clampGraphValue(const qreal &value);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
    stdsptr<Key> readKey(QIODevice *target);
public slots:
    void anim_saveCurrentValueAsKey();
protected:
    void graph_getValueConstraints(GraphKey *key, const QrealPointType& type,
                                   qreal &minMoveValue, qreal &maxMoveValue) const;
public:
    Snapshot makeSnapshot() const;

    void setPrefferedValueStep(const qreal &valueStep);

    void setValueRange(const qreal &minVal, const qreal &maxVal);
    void setMinValue(const qreal &minVal) {
        mMinPossibleVal = minVal;
        setCurrentBaseValue(mCurrentBaseValue);
    }
    void setMaxValue(const qreal &maxVal) {
        mMaxPossibleVal = maxVal;
        setCurrentBaseValue(mCurrentBaseValue);
    }

    void setCurrentBaseValue(qreal newValue);
    void setCurrentBaseValueNoUpdate(const qreal &newValue);
    void incCurrentBaseValue(const qreal &incBy);
    void incCurrentValueNoUpdate(const qreal &incBy);
    void multCurrentBaseValue(const qreal &mult);

    qreal getCurrentBaseValue() const;
    qreal getCurrentEffectiveValue() const;
    qreal getBaseValue(const qreal &relFrame) const;
    qreal getBaseValueAtAbsFrame(const qreal &frame) const;
    qreal getEffectiveValue(const qreal &relFrame) const;
    qreal getEffectiveValueAtAbsFrame(const qreal &frame) const;

    qreal getSavedBaseValue();
    void incAllValues(const qreal &valInc);

    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    qreal getPrefferedValueStep();

    void graphFixMinMaxValues();

    void saveValueToKey(const int &frame, const qreal &value);
    void removeThisFromGraphAnimator();

    void incSavedValueToCurrentValue(const qreal &incBy);
    void multSavedValueToCurrentValue(const qreal &multBy);

    const int &getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(const int &decimals) { mDecimals = decimals; }

    bool getBeingTransformed() { return mTransformed; }

    void setGenerator(const qsptr<RandomQrealGenerator> &generator);

    bool hasNoise();

    static auto create0to1Animator(const QString& name) {
        auto anim = SPtrCreate(QrealAnimator)(0, 0, 1, 0.01, name);
        anim->graphFixMinMaxValues();
        return anim;
    }
private:
    qreal calculateBaseValueAtRelFrame(const qreal &frame) const;

    bool mGraphMinMaxValuesFixed = false;
    bool mTransformed = false;

    int mDecimals = 3;

    qreal mMaxPossibleVal = TEN_MIL;
    qreal mMinPossibleVal = -TEN_MIL;

    qreal mCurrentBaseValue = 0;
    qreal mSavedCurrentValue = 0;

    qsptr<RandomQrealGenerator> mRandomGenerator;

    qreal mPrefferedValueStep = 1;
    bool updateBaseValueFromCurrentFrame();
signals:
    void valueChangedSignal(qreal);
};

#endif // VALUEANIMATORS_H
