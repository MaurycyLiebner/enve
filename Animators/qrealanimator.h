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

const int KEY_RECT_SIZE = 10;

class ComplexAnimator;
class QrealAnimatorValueSlider;

#include <QDoubleSpinBox>

class QrealAnimator :  public Animator
{
    Q_OBJECT
public:
    QrealAnimator();

    ~QrealAnimator();

    void addSlider(QrealAnimatorValueSlider *valueSlider);
    void removeSlider(QrealAnimatorValueSlider *valueSlider);

    void setValueRange(qreal minVal, qreal maxVal);

    qreal getValueAtFrame(int frame) const;
    qreal getCurrentValue() const;
    void setCurrentValue(qreal newValue, bool finish = false);
    void updateValueFromCurrentFrame();
    void saveCurrentValueToKey(QrealKey *key);
    void saveValueToKey(QrealKey *key,
                        qreal value,
                        bool saveUndoRedo = true);

    virtual void setFrame(int frame);
    QrealKey *getKeyAtFrame(int frame);
    void saveCurrentValueAsKey();
    virtual void updateKeysPath();
    void appendKey(QrealKey *newKey,
                   bool saveUndoRedo = true);
    void removeKey(QrealKey *removeKey,
                   bool saveUndoRedo = true);

    void moveKeyToFrame(QrealKey *key, int newFrame);

    virtual void sortKeys();
    bool getNextAndPreviousKeyId(int *prevIdP,
                                 int *nextIdP,
                                 int frame) const;

    void mousePress(QPointF pressPos);
    void mouseMove(QPointF mousePos);
    void mouseRelease();

    void updateDrawPathIfNeeded(qreal height, qreal margin,
                        qreal startFrame, qreal minShownVal,
                        qreal pixelsPerFrame, qreal pixelsPerValUnit);
    void updateDimensions();

    void setScale(qreal scale);
    void incScale(qreal inc);
    void mergeKeysIfNeeded();
    void clearKeysSelection();

    void addKeyToSelection(QrealKey *key);
    void removeKeyFromSelection(QrealKey *key);
    void setViewedFramesRange(int minF, int maxF);
    void getMinAndMaxValues(qreal *minValP, qreal *maxValP);
    void getMinAndMaxValuesBetweenFrames(int startFrame, int endFrame,
                                         qreal *minValP, qreal *maxValP);
    void setMinShownVal(qreal newMinShownVal);
    void incMinShownVal(qreal inc);
    QrealPoint *getPointAt(qreal value, qreal frame,
                           qreal pixelsPerFrame, qreal pixelsPerValUnit);
    void deletePressed();
    qreal getValueAtFrame(int frame,
                          QrealKey *prevKey,
                          QrealKey *nextKey) const;

    void middlePress(QPointF pressPos);
    void middleMove(QPointF movePos);
    void middleRelease();

    int getStartFrame();
    int getEndFrame();

    void constrainCtrlsFrameValues();

    virtual qreal clampValue(qreal value);

    qreal getPrevKeyValue(QrealKey *key);
    qreal getNextKeyValue(QrealKey *key);

    bool hasPrevKey(QrealKey *key);
    bool hasNextKey(QrealKey *key);

    virtual void retrieveSavedValue();
    void incCurrentValue(qreal incBy);

    virtual void startTransform();

    virtual void finishTransform();

    void multCurrentValue(qreal mult);

    qreal getSavedValue();

    virtual void cancelTransform();

    virtual void setUpdater(AnimatorUpdater *updater);

    virtual void callUpdater();
    virtual void callSoftUpdater();

    virtual void drawKeys(QPainter *p, qreal pixelsPerFrame, qreal startY,
                          int startFrame, int endFrame);
    virtual void getKeysInRect(QRectF selectionRect,
                               int minViewedFrame,
                               qreal pixelsPerFrame,
                               QList<QrealKey *> *keysList);

    void incAllValues(qreal valInc);

    QString getName();
    void setName(QString newName);

    void updateKeyOnCurrrentFrame();
    bool isKeyOnCurrentFrame();

    virtual void setRecording(bool rec);
    bool isRecording();
    void removeAllKeys();

    virtual bool isDescendantRecording() { return mIsRecording; }
    virtual QString getValueText();
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

    qreal getPrefferedValueStep();
    void setPrefferedValueStep(qreal valueStep);
    virtual void clearFromGraphView();

    void freezeMinMaxValues();
    void saveValueAtFrameAsKey(int frame);

    void setTraceKeyOnCurrentFrame(bool bT);
    virtual void openContextMenu(QPoint pos);
    void saveValueToKey(int frame, qreal value);
    void removeThisFromGraphAnimator();

    void setAnimatorColor(QColor color);
    virtual int saveToSql(QSqlQuery *query);
    void loadFromSql(int qrealAnimatorId);
    void loadKeysFromSql(int qrealAnimatorId);
    virtual void incSavedValueToCurrentValue(qreal incBy);
    virtual void multSavedValueToCurrentValue(qreal multBy);

    bool isCurrentAnimator() { return mIsCurrentAnimator; }
    const QColor &getAnimatorColor() { return mAnimatorColor; }

    QrealKey *getKeyAtPos(qreal relX, int minViewedFrame,
                          qreal pixelsPerFrame);
    void removeAllKeysFromComplexAnimator();
    void addAllKeysToComplexAnimator();

    bool hasKeys();
    void setRecordingWithoutChangingKeys(bool rec,
                                         bool saveUndoRedo = true);
    virtual void setTransformed(bool bT);

    int getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(int decimals) { mDecimals = decimals; }

    virtual void makeDuplicate(QrealAnimator *target);
    virtual QrealAnimator *makeDuplicate();

    SingleWidgetTargetType SWT_getType() { return SWT_QrealAnimator; }
protected:
    bool mTraceKeyOnCurrentFrame = false;

    bool mMinMaxValuesFrozen = false;

    bool mDrawPathUpdateNeeded = false;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = -DBL_MAX;
    AnimatorUpdater *mUpdater = NULL;
    bool mTransformed = false;

    qreal mCurrentValue = 0.;
    qreal mSavedCurrentValue;
    QList<QrealKey*> mKeys;
    QPainterPath mKeysPath;
    QPainterPath mKeysDrawPath;

    int mCurrentFrame = 0;

    qreal mPrefferedValueStep = 1.;

    QColor mAnimatorColor;

    int mDecimals = 3;
signals:
    void valueChangedSignal(qreal);
};

#endif // VALUEANIMATORS_H
