#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include "float.h"
#include "animator.h"
class QrealKey;
class QrealPoint;
class QrealAnimatorValueSlider;

#define Q_FOREACHQK(key, keysList) Q_FOREACH(Key *keyK, keysList) { key = (QrealKey*)keyK;

#include <QDoubleSpinBox>

class QrealAnimator :  public Animator
{
    Q_OBJECT
public:
    QrealAnimator();

    virtual ~QrealAnimator();

    void qra_addSlider(QrealAnimatorValueSlider *valueSlider);
    void qra_removeSlider(QrealAnimatorValueSlider *valueSlider);

    void qra_setValueRange(const qreal &minVal,
                           const qreal &maxVal);

    qreal qra_getValueAtAbsFrame(const int &frame) const;
    qreal qra_getCurrentValue() const;
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
    void anim_appendKey(Key *newKey,
                        const bool &saveUndoRedo = true);
    void anim_removeKey(Key *keyToRemove,
                        const bool &saveUndoRedo = true);
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo,
                                const bool &finish);

    qreal getPrefferedValueStep();
    virtual void setPrefferedValueStep(const qreal &valueStep);
    virtual void prp_clearFromGraphView();

    void freezeMinMaxValues();
    void saveValueAtAbsFrameAsKey(const int &frame);

    virtual void prp_openContextMenu(const QPoint &pos);
    void qra_saveValueToKey(const int &frame, const qreal &value);
    void removeThisFromGraphAnimator();

    int saveToSql(QSqlQuery *query, const int &parentId = 0);
    void loadFromSql(const int &qrealAnimatorId);
    void anim_loadKeysFromSql(const int &qrealAnimatorId);
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

    void makeDuplicate(Property *target);
    Property *makeDuplicate();

    qreal getCurrentValueAtAbsFrame(const int &frame) const;
    qreal getCurrentValueAtRelFrame(const int &frame) const;
    qreal qra_getValueAtRelFrame(const int &frame) const;
    void prp_setTransformed(const bool &bT) { mTransformed = bT; }
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

    bool SWT_isQrealAnimator() { return true; }
protected:
    std::map<void*, QColor> mAnimatorColors;

    bool mMinMaxValuesFrozen = false;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = -DBL_MAX;
    bool mTransformed = false;

    qreal mCurrentValue = 0.;
    qreal mSavedCurrentValue = 0.;
    QPainterPath mKeysPath;

    qreal mPrefferedValueStep = 1.;

    QColor anim_mAnimatorColor;

    int mDecimals = 3;
signals:
    void valueChangedSignal(qreal);
public slots:
    void prp_setRecording(const bool &rec);

    void anim_saveCurrentValueAsKey();
};

#endif // VALUEANIMATORS_H
