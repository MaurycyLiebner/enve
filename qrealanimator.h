#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainterPath>
#include <QPainter>

class QrealPoint;

class QrealKey
{
public:
    QrealKey(int frame);
    QrealPoint *mousePress(qreal frameT, qreal valueT,
                    qreal pixelsPerFrame, qreal pixelsPerValue);

    bool isSelected();

    qreal getValue();
    void setValue(qreal value);
    void setStartValue(qreal value);
    void setEndValue(qreal value);

    int getFrame();
    void setFrame(int frame);
    void setStartFrame(qreal startFrame);
    void setEndFrame(qreal endFrame);

    qreal getStartValue();
    qreal getEndValue();
    qreal getStartValueFrame();
    qreal getEndValueFrame();

    void setStartEnabled(bool bT);

    void setEndEnabled(bool bT);

    void setSelected(bool bT);

    bool isInsideRect(QRectF valueFrameRect);

    void draw(QPainter *p,
              qreal minFrameT, qreal minValueT,
              qreal pixelsPerFrame, qreal pixelsPerValue);
    void changeFrameAndValueBy(QPointF frameValueChange);
    void saveCurrentFrameAndValue();
private:
    QrealPoint *mKeyPoint;
    QrealPoint *mStartPoint;
    QrealPoint *mEndPoint;

    qreal mValue;
    int mFrame;
    qreal mSavedFrame;
    qreal mSavedValue;

    qreal mSavedMaxStartFrameDist;
    qreal mSavedMaxEndFrameDist;

    qreal mStartValue = 0.;
    qreal mEndValue = 0.;
    qreal mStartFrame = 0.;
    qreal mEndFrame = 0.;
    bool mStartEnabled = false;
    bool mEndEnabled = false;
};

enum QrealPointType {
    START_POINT,
    END_POINT,
    KEY_POINT
};

class QrealPoint
{
public:
    QrealPoint(QrealPointType type, QrealKey *parentKey);

    qreal getFrame();

    void setFrame(qreal frame);

    qreal getValue();

    void setValue(qreal value);

    bool isSelected();

    bool isNear(qreal frameT, qreal valueT,
                qreal pixelsPerFrame, qreal pixelsPerValue);

    void moveTo(qreal frameT, qreal valueT);

    void draw(QPainter *p,
              qreal minFrameT, qreal minValueT,
              qreal pixelsPerFrame, qreal pixelsPerValue);

    void setSelected(bool bT);

    bool isKeyPoint();
    bool isStartPoint();
    bool isEndPoint();

    QrealKey *getParentKey();
private:
    bool mIsSelected = false;
    QrealPointType mType;
    QrealKey *mParentKey;
    qreal mRadius = 10.f;
};

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
    void getKeyMultiplicatorsAtFrame(int frame,
                                     QrealKey *prevKey,
                                     QrealKey *nextKey,
                                     qreal *prevMultiplicator,
                                     qreal *nextMultiplicator);

    void draw(QPainter *p);
    void mousePress(QPointF pressPos);
    void mouseMove(QPointF mousePos);
    void mouseRelease();

    void updateDrawPath();
    void updateDimensions();

    void updateMinAndMaxMove(QrealKey *key);
    void setMargin(qreal margin);
    void incMargin(qreal inc);
    void mergeKeysIfNeeded();
    void clearKeysSelection();

    void addKeyToSelection(QrealKey *key);
    void removeKeyFromSelection(QrealKey *key);
    void setViewedFramesRange(int minF, int maxF);
    void setRect(QRectF rect);
    void getMinAndMaxValues(qreal *minValP, qreal *maxValP);
private:
    qreal mMargin = 20.f;

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
