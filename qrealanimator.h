#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include "qrealkey.h"
#include "connectedtomainwindow.h"
#include "animatorupdater.h"
#include "float.h"

const int KEY_RECT_SIZE = 10;

class ComplexAnimator;

#include <QDoubleSpinBox>

class QrealAnimator : public ConnectedToMainWindow
{
public:
    QrealAnimator();

    void setValueRange(qreal minVal, qreal maxVal);

    void setParentAnimator(ComplexAnimator *parentAnimator);

    qreal getValueAtFrame(int frame) const;
    qreal getCurrentValue() const;
    void setCurrentValue(qreal newValue, bool finish = false);
    void updateValueFromCurrentFrame();
    void saveCurrentValueToKey(QrealKey *key);
    void saveValueToKey(QrealKey *key, qreal value);

    virtual void setFrame(int frame);
    QrealKey *getKeyAtFrame(int frame);
    void saveCurrentValueAsKey();
    virtual void updateKeysPath();
    void appendKey(QrealKey *newKey);
    void removeKey(QrealKey *removeKey);

    void moveKeyToFrame(QrealKey *key, int newFrame);

    virtual void sortKeys();
    bool getNextAndPreviousKeyId(int *prevIdP, int *nextIdP, int frame) const;

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
    qreal getValueAtFrame(int frame, QrealKey *prevKey, QrealKey *nextKey) const;

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

    virtual void drawKeys(QPainter *p, qreal pixelsPerFrame,
                          qreal startX, qreal startY, qreal height,
                          int startFrame, int endFrame);
    virtual QrealKey *getKeyAtPos(qreal relX, qreal relY,
                                  int minViewedFrame, qreal pixelsPerFrame);
    virtual void getKeysInRect(QRectF selectionRect,
                               int minViewedFrame,
                               qreal pixelsPerFrame,
                               QList<QrealKey *> *keysList);

    void addAllKeysToComplexAnimator();
    void removeAllKeysFromComplexAnimator();

    bool hasKeys();

    void incAllValues(qreal valInc);

    QString getName();
    void setName(QString newName);

    void updateKeyOnCurrrentFrame();
    bool isKeyOnCurrentFrame();

    bool isBoxesListDetailVisible();
    void setBoxesListDetailVisible(bool bT);
    virtual qreal getBoxesListHeight();

    virtual void drawBoxesList(QPainter *p,
                               qreal drawX, qreal drawY);

    virtual void handleListItemMousePress(qreal boxesListX,
                                          qreal relX, qreal relY,
                                          QMouseEvent *event);

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
protected:
    bool mTraceKeyOnCurrentFrame = false;

    bool mMinMaxValuesFrozen = false;

    bool mIsCurrentAnimator = false;
    bool mDrawPathUpdateNeeded = false;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = -DBL_MAX;

    bool mIsRecording = false;
    bool mBoxesListDetailVisible = false;
    bool mKeyOnCurrentFrame = false;

    QString mName = "";

    ComplexAnimator *mParentAnimator = NULL;

    AnimatorUpdater *mUpdater = NULL;

    bool mTransformed = false;

    qreal mCurrentValue;
    qreal mSavedCurrentValue;
    QList<QrealKey*> mKeys;
    QPainterPath mKeysPath;
    QPainterPath mKeysDrawPath;

    int mCurrentFrame = 0;

    bool mIsComplexAnimator = false;

    qreal mPrefferedValueStep = 1.;
};
#include <QMenu>
class QrealAnimatorSpin : public QMenu
{
    Q_OBJECT
public:
    QrealAnimatorSpin(QrealAnimator *animator);
public slots:
    void valueEdited(double newVal);
    void finishValueEdit();
private:
    QrealAnimator *mAnimator;
};

#endif // VALUEANIMATORS_H
