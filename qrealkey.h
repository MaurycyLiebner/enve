#ifndef QREALKEY_H
#define QREALKEY_H
#include "qrealpoint.h"
#include "pointhelpers.h"

class QrealAnimator;

class QrealKey : public QrealPoint
{
public:
    QrealKey(int frame, QrealAnimator *parentAnimator, qreal value = 0.);
    QrealPoint *mousePress(qreal frameT, qreal valueT,
                    qreal pixelsPerFrame, qreal pixelsPerValue);
    virtual ~QrealKey();

    void updateCtrlFromCtrl(QrealPointType type);

    virtual qreal getValue();
    virtual void setValue(qreal value);
    virtual void setStartValue(qreal value);
    virtual void setEndValue(qreal value);

    int getFrame();
    virtual void setFrame(int frame);
    virtual void setStartFrame(qreal startFrame);
    virtual void setEndFrame(qreal endFrame);

    qreal getStartValue();
    qreal getEndValue();
    qreal getStartValueFrame();
    qreal getEndValueFrame();

    void setStartEnabled(bool bT);

    void setEndEnabled(bool bT);

    bool isInsideRect(QRectF valueFrameRect);

    void draw(QPainter *p,
              qreal minFrameT, qreal minValueT,
              qreal pixelsPerFrame, qreal pixelsPerValue);
    void changeFrameAndValueBy(QPointF frameValueChange);
    void saveCurrentFrameAndValue();
    void setCtrlsMode(CtrlsMode mode);

    void constrainStartCtrlMinFrame(int minFrame);
    void constrainEndCtrlMaxFrame(int maxFrame);

    QrealPoint *getStartPoint();
    QrealPoint *getEndPoint();

    bool isEndPointEnabled();
    bool isStartPointEnabled();

    qreal getPrevKeyValue();
    qreal getNextKeyValue();

    bool hasPrevKey();
    bool hasNextKey();
    void setFrameAndUpdateParentAnimator(int newFrame);
    void incFrameAndUpdateParentAnimator(int inc);

    QrealAnimator *getParentAnimator();
protected:
    QrealAnimator *mParentAnimator;

    CtrlsMode mCtrlsMode = CTRLS_SYMMETRIC;

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

#endif // QREALKEY_H
