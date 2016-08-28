#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainterPath>
#include <QPainter>
#include "qrealkey.h"

class QrealAnimator
{
public:
    QrealAnimator();

    qreal getValueAtFrame(int frame);
    qreal getCurrentValue();
    void setCurrentValue(qreal newValue);
    void updateValueFromCurrentFrame();
    QrealKey *addNewKeyAtFrame(int frame);
    void saveCurrentValueToKey(QrealKey *key);
    void saveValueToKey(QrealKey *key, qreal value);

    void setFrame(int frame);
    QrealKey *getKeyAtFrame(int frame);
    void saveCurrentValueAsKey();
    void updateKeysPath();
    void appendKey(QrealKey *newKey);
    void removeKey(QrealKey *removeKey);
    void sortKeys();
    bool getNextAndPreviousKeyId(int *prevIdP, int *nextIdP, int frame);

    void draw(QPainter *p);

    void mousePress(QPointF pressPos);
    void mouseMove(QPointF mousePos);
    void mouseRelease();

    void updateDrawPath();
    void updateDimensions();

    virtual void updateMinAndMaxMove(QrealKey *key);
    void setScale(qreal scale);
    void incScale(qreal inc);
    void mergeKeysIfNeeded();
    void clearKeysSelection();

    void addKeyToSelection(QrealKey *key);
    void removeKeyFromSelection(QrealKey *key);
    void setViewedFramesRange(int minF, int maxF);
    void setRect(QRectF rect);
    void getMinAndMaxValues(qreal *minValP, qreal *maxValP);
    void setMinShownVal(qreal newMinShownVal);
    void incMinShownVal(qreal inc);
    QrealPoint *getPointAt(qreal value, qreal frame);
    QrealPoint *getPointAt(QPointF pos);
    void deletePressed();
    qreal getValueAtFrame(int frame, QrealKey *prevKey, QrealKey *nextKey);

    void middlePress(QPointF pressPos);
    void middleMove(QPointF movePos);
    void middleRelease();

    int getStartFrame();
    int getEndFrame();

    void setTwoSideCtrlForSelected();
    void setRightSideCtrlForSelected();
    void setLeftSideCtrlForSelected();
    void setNoSideCtrlForSelected();
    void setCtrlsModeForSelected(CtrlsMode mode);
    void constrainCtrlsFrameValues();

    virtual qreal clampValue(qreal value);

    qreal getPrevKeyValue(QrealKey *key);
    qreal getNextKeyValue(QrealKey *key);

    bool hasPrevKey(QrealKey *key);
    bool hasNextKey(QrealKey *key);
protected:
    int mSavedStartFrame;
    int mSavedEndFrame;
    qreal mSavedMinShownValue;
    QPointF mMiddlePressPos;

    qreal mMargin = 20.;
    qreal mValueScale = 1.;
    qreal mMinShownVal = 0.;

    QPointF mPressFrameAndValue;

    QRectF mSelectionRect;
    bool mSelecting = false;

    bool mFirstMove = false;

    QrealPoint *mCurrentPoint = NULL;
    QList<QrealKey*> mSelectedKeys;

    qreal mCurrentValue;
    QList<QrealKey*> mKeys;
    QPainterPath mKeysPath;
    QPainterPath mKeysDrawPath;

    qreal mPixelsPerValUnit;
    qreal mPixelsPerFrame;
    QRectF mDrawRect;
    int mStartFrame;
    int mEndFrame;
    qreal mMinVal;
    qreal mMaxVal;

    qreal mMaxMoveFrame;
    qreal mMinMoveFrame;

    int mCurrentFrame = 0;
    void getMinAndMaxValuesBetweenFrames(int startFrame, int endFrame,
                                         qreal *minValP, qreal *maxValP);
    void getValueAndFrameFromPos(QPointF pos, qreal *value, qreal *frame);
    void addKeysInRectToSelection(QRectF rect);
};

#endif // VALUEANIMATORS_H
