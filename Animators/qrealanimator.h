#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include "qrealkey.h"
#include "connectedtomainwindow.h"
#include "Animators/animatorupdater.h"
#include "float.h"
#include "animator.h"

class DurationRectangleMovable;

const int KEY_RECT_SIZE = 12;

class ComplexAnimator;
class QrealAnimatorValueSlider;

#define foreachQK(key, keysList) foreach(Key *keyK, keysList) { key = (QrealKey*)keyK;

#include <QDoubleSpinBox>

class QrealAnimator :  public Animator
{
    Q_OBJECT
public:
    QrealAnimator();

    virtual ~QrealAnimator();

    void qra_addSlider(QrealAnimatorValueSlider *valueSlider);
    void qra_removeSlider(QrealAnimatorValueSlider *valueSlider);

    void qra_setValueRange(qreal minVal, qreal maxVal);

    qreal qra_getValueAtAbsFrame(int frame) const;
    qreal qra_getCurrentValue() const;
    void qra_setCurrentValue(qreal newValue, bool finish = false);
    void qra_updateValueFromCurrentFrame();
    void qra_saveCurrentValueToKey(QrealKey *key);
    void qra_saveValueToKey(QrealKey *key,
                        qreal value,
                        bool saveUndoRedo = true);

    virtual void prp_setAbsFrame(int frame);
    virtual void qra_updateKeysPath();


    void qra_updateDrawPathIfNeeded(
                        qreal height, qreal margin,
                        qreal startFrame, qreal minShownVal,
                        qreal pixelsPerFrame, qreal pixelsPerValUnit);

    void qra_getMinAndMaxValues(qreal *minValP, qreal *maxValP);
    void qra_getMinAndMaxValuesBetweenFrames(int startFrame, int endFrame,
                                             qreal *minValP, qreal *maxValP);

    QrealPoint *qra_getPointAt(qreal value, qreal frame,
                               qreal pixelsPerFrame,
                               qreal pixelsPerValUnit);
    qreal qra_getValueAtRelFrame(int frame,
                          QrealKey *prevKey,
                          QrealKey *nextKey) const;

    void qra_constrainCtrlsFrameValues();

    virtual qreal qra_clampValue(qreal value);

    qreal qra_getPrevKeyValue(QrealKey *key);
    qreal qra_getNextKeyValue(QrealKey *key);

    virtual void prp_retrieveSavedValue();
    void qra_incCurrentValue(qreal incBy);

    virtual void prp_startTransform();

    virtual void prp_finishTransform();

    void qra_multCurrentValue(qreal mult);

    qreal qra_getSavedValue();

    virtual void prp_cancelTransform();  

    void qra_incAllValues(qreal valInc);

    virtual QString prp_getValueText();
    void getMinAndMaxMoveFrame(QrealKey *key, QrealPoint *currentPoint,
                               qreal *minMoveFrame, qreal *maxMoveFrame);
    void drawKeysPath(QPainter *p,
                      qreal height, qreal margin,
                      qreal startFrame, qreal minShownVal,
                      qreal pixelsPerFrame, qreal pixelsPerValUnit);
    void setDrawPathUpdateNeeded();
    void addKeysInRectToList(QRectF frameValueRect,
                             QList<QrealKey*> *keys);

    void setIsCurrentAnimator(bool bT);
    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    void anim_mergeKeysIfNeeded();
    void anim_appendKey(Key *newKey,
                       bool saveUndoRedo = true);
    void anim_removeKey(Key *keyToRemove,
                        bool saveUndoRedo = true);
    void anim_moveKeyToFrame(Key *key, int newFrame);

    qreal getPrefferedValueStep();
    virtual void setPrefferedValueStep(const qreal &valueStep);
    virtual void prp_clearFromGraphView();

    void freezeMinMaxValues();
    void saveValueAtAbsFrameAsKey(int frame);

    virtual void prp_openContextMenu(QPoint pos);
    void qra_saveValueToKey(int frame, qreal value);
    void removeThisFromGraphAnimator();

    void setAnimatorColor(QColor color);
    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);
    void prp_loadFromSql(const int &qrealAnimatorId);
    void anim_loadKeysFromSql(int qrealAnimatorId);
    virtual void incSavedValueToCurrentValue(qreal incBy);
    virtual void multSavedValueToCurrentValue(qreal multBy);

    const QColor &getAnimatorColor() const { return anim_mAnimatorColor; }

    int getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(int decimals) { mDecimals = decimals; }

    void prp_makeDuplicate(Property *target);
    Property *prp_makeDuplicate();

    SWT_Type SWT_getType() { return SWT_QrealAnimator; }
    qreal getCurrentValueAtAbsFrame(const int &frame) const;
    qreal qra_getValueAtRelFrame(int frame) const;
    void prp_setTransformed(bool bT) { mTransformed = bT; }
    void anim_removeAllKeys();
    QrealKey *getQrealKeyAtId(const int &id) const;


protected:
    bool mMinMaxValuesFrozen = false;

    bool mDrawPathUpdateNeeded = false;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = -DBL_MAX;
    bool mTransformed = false;

    qreal mCurrentValue = 0.;
    qreal mSavedCurrentValue;
    QPainterPath mKeysPath;
    QPainterPath mKeysDrawPath;

    qreal mPrefferedValueStep = 1.;

    QColor anim_mAnimatorColor;

    int mDecimals = 3;
signals:
    void valueChangedSignal(qreal);
public slots:
    void prp_setRecording(bool rec);

    void anim_saveCurrentValueAsKey();
};

#endif // VALUEANIMATORS_H
