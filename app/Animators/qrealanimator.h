#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include "float.h"
#include "Animators/animator.h"
class QrealKey;
class QrealPoint;
class QrealAnimatorValueSlider;
class RandomQrealGenerator;
#define Q_FOREACHQK(key, keysList) Q_FOREACH(Key *keyK, keysList) { key = GetAsPtr(keyK, QrealKey);

#include <QDoubleSpinBox>

class QrealAnimator :  public Animator {
    Q_OBJECT
    friend class SelfRef;
public:
    virtual ~QrealAnimator();

    void qra_setValueRange(const qreal &minVal,
                           const qreal &maxVal);

    qreal qra_getValueAtAbsFrame(const int &frame);
    qreal qra_getCurrentValue() const;
    qreal qra_getCurrentEffectiveValue();
    void qra_setCurrentValue(qreal newValue,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false,
                             const bool &callUpdater = true);
    void qra_updateValueFromCurrentFrame();
    void qra_saveCurrentValueToKey(QrealKey *key,
                                   const bool &finish = true);
    void qra_saveValueToKey(QrealKey *key,
                            const qreal &value,
                            const bool &saveUndoRedo = true,
                            const bool &finish = true);

    void prp_setAbsFrame(const int &frame);
    virtual void qra_updateKeysPath();

    void qra_getMinAndMaxValues(qreal *minValP, qreal *maxValP);
    void qra_getMinAndMaxValuesBetweenFrames(const int &startFrame,
                                             const int &endFrame,
                                             qreal *minValP,
                                             qreal *maxValP);

    QrealPoint *qra_getPointAt(const qreal &value,
                               const qreal &frame,
                               const qreal &pixelsPerFrame,
                               const qreal &pixelsPerValUnit);
    qreal qra_getValueAtRelFrame(const int &frame,
                                 QrealKey *prevKey,
                                 QrealKey *nextKey) const;

    void qra_constrainCtrlsFrameValues();

    qreal qra_clampValue(const qreal &value);

    qreal qra_getPrevKeyValue(QrealKey *key);
    qreal qra_getNextKeyValue(QrealKey *key);
    int qra_getPrevKeyRelFrame(QrealKey *key);
    int qra_getNextKeyRelFrame(QrealKey *key);

    virtual void prp_retrieveSavedValue();
    void qra_incCurrentValue(const qreal &incBy,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false,
                             const bool &callUpdater = true);

    virtual void prp_startTransform();

    void prp_finishTransform();

    void qra_multCurrentValue(const qreal &mult);

    qreal qra_getSavedValue();

    virtual void prp_cancelTransform();  

    void qra_incAllValues(const qreal &valInc,
                          const bool &saveUndoRedo = false,
                          const bool &finish = false,
                          const bool &callUpdater = true);

    virtual QString prp_getValueText();
    void getMinAndMaxMoveFrame(QrealKey *key,
                               QrealPoint *currentPoint,
                               qreal *minMoveFrame,
                               qreal *maxMoveFrame);
    void drawKeysPath(QPainter *p,
                      const QColor &paintColor);

    void addKeysInRectToList(const QRectF &frameValueRect,
                             QList<QrealKey*> *keys);

    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    void anim_mergeKeysIfNeeded();
    void anim_appendKey(const stdsptr<Key> &newKey,
                        const bool &saveUndoRedo = true,
                        const bool &update = true);
    void anim_removeKey(const stdsptr<Key> &keyToRemove,
                        const bool &saveUndoRedo = true);
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo,
                                const bool &finish);

    qreal getPrefferedValueStep();
    virtual void setPrefferedValueStep(const qreal &valueStep);
    virtual void prp_clearFromGraphView();

    void graphFixMinMaxValues();
    void saveValueAtAbsFrameAsKey(const int &frame);

    virtual void prp_openContextMenu(const QPoint &pos);
    void qra_saveValueToKey(const int &frame, const qreal &value);
    void removeThisFromGraphAnimator();

    void incSavedValueToCurrentValue(const qreal &incBy);
    void multSavedValueToCurrentValue(const qreal &multBy);

    QColor getAnimatorColor(void *ptr) const {
        for(const std::map<void*, QColor>::value_type& x : mAnimatorColors) {
            if(x.first == ptr) {
                return x.second;
            }
        }
        return QColor();
    }

    void setAnimatorColor(void *ptr, const QColor &color) {
        mAnimatorColors[ptr] = color;
    }
    void removeAnimatorColor(void *ptr) {
        mAnimatorColors.erase(ptr);
    }

    bool isCurrentAnimator(void *ptr) const {
        return mAnimatorColors.find(ptr) != mAnimatorColors.end();
    }

    const int &getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(const int &decimals) { mDecimals = decimals; }

    qreal getCurrentValueAtAbsFrame(const int &frame);
    qreal getCurrentValueAtRelFrame(const int &frame) const;
    qreal qra_getValueAtRelFrame(const int &frame) const;
    void prp_setTransformed(const bool &bT) { mTransformed = bT; }
    bool getBeingTransformed() { return mTransformed; }
    void anim_removeAllKeys();
    QrealKey *getQrealKeyAtId(const int &id) const;

    void prp_updateAfterChangedRelFrameRange(const int &minFrame,
                                             const int &maxFrame) {
        if(anim_mCurrentRelFrame >= minFrame) {
            if(anim_mCurrentRelFrame <= maxFrame) {
                qra_updateValueFromCurrentFrame();
            }
        }
        Property::prp_updateAfterChangedRelFrameRange(minFrame, maxFrame);
    }

    void setGenerator(const qsptr<RandomQrealGenerator> &generator);

    bool SWT_isQrealAnimator() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
    stdsptr<Key> readKey(QIODevice *target);
    bool qra_hasNoise();

    qreal qra_getEffectiveValueAtRelFrame(const int &frame) const;
    qreal qra_getEffectiveValueAtAbsFrame(const int &frame);
    qreal getCurrentEffectiveValueAtRelFrame(const int &frame) const;
    qreal getCurrentEffectiveValueAtAbsFrame(const int &frame);
    qreal qra_getValueAtRelFrameF(const qreal &frame) const;
    qreal qra_getEffectiveValueAtRelFrameF(const qreal &frame) const;
    qreal getCurrentEffectiveValueAtRelFrameF(const qreal &frame) const;
    qreal getCurrentEffectiveValueAtAbsFrameF(const qreal &frame);
    qreal qra_getValueAtRelFrameF(const qreal &frame,
                                  QrealKey *prevKey,
                                  QrealKey *nextKey) const;
    qreal getCurrentValueAtRelFrameF(const qreal &frame) const;
    qreal getCurrentValueAtAbsFrameF(const qreal &frame);

    static auto create0to1Animator(const QString& name) {
        auto anim = SPtrCreate(QrealAnimator)(0., 0., 1., 0.01, name);
        anim->graphFixMinMaxValues();
        return anim;
    }
protected:
    QrealAnimator(const QString& name);
    QrealAnimator(const qreal &iniVal,
                  const qreal &minVal,
                  const qreal &maxVal,
                  const qreal &prefferdStep,
                  const QString& name);

    bool mGraphMinMaxValuesFixed = false;
    bool mTransformed = false;

    int mDecimals = 3;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = -DBL_MAX;

    qreal mCurrentValue = 0.;
    qreal mSavedCurrentValue = 0.;

    qsptr<RandomQrealGenerator> mRandomGenerator;
    std::map<void*, QColor> mAnimatorColors;

    QPainterPath mKeysPath;

    qreal mPrefferedValueStep = 1.;
signals:
    void valueChangedSignal(qreal);
public slots:
    void prp_setRecording(const bool& rec);

    void anim_saveCurrentValueAsKey();
};

#endif // VALUEANIMATORS_H
