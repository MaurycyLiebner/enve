#ifndef QREALKEY_H
#define QREALKEY_H
#include "key.h"
#include "pointhelpers.h"

class QPainter;
class ComplexAnimator;

class ComplexKey;

class QrealAnimator;
class KeysClipboardContainer;
class QrealPoint;
enum QrealPointType : short;

class QrealKey : public Key {
public:
    QrealKey(QrealAnimator *parentAnimator);
    QrealKey(const int &frame,
             const qreal &val,
             QrealAnimator *parentAnimator);
    QrealPoint *mousePress(qreal frameT, qreal valueT,
                    qreal pixelsPerFrame, qreal pixelsPerValue);
    virtual ~QrealKey();


    Key *makeKeyDuplicate(Animator *animator);
    QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    void updateCtrlFromCtrl(QrealPointType type);

    virtual qreal getValue();
    virtual void setValue(qreal value, bool saveUndoRedo = true);
    virtual void setStartValue(qreal value);
    virtual void setEndValue(qreal value);

    virtual void setStartFrame(qreal startFrame);
    virtual void setEndFrame(qreal endFrame);

    qreal getStartValue();
    qreal getEndValue();
    qreal getStartValueFrame();
    qreal getEndValueFrame();

    virtual void setStartEnabled(bool bT);

    virtual void setEndEnabled(bool bT);

    bool isInsideRect(QRectF valueFrameRect);

    void drawGraphKey(QPainter *p,
              qreal minFrameT, qreal minValueT,
              qreal pixelsPerFrame, qreal pixelsPerValue);
    void changeFrameAndValueBy(QPointF frameValueChange);
    void saveCurrentFrameAndValue();
    virtual void setCtrlsMode(CtrlsMode mode);

    void constrainStartCtrlMinFrame(int minFrame);
    void constrainEndCtrlMaxFrame(int maxFrame);

    QrealPoint *getStartPoint();
    QrealPoint *getEndPoint();
    QrealPoint *getGraphPoint();

    bool isEndPointEnabled();
    bool isStartPointEnabled();

    qreal getPrevKeyValue();
    qreal getNextKeyValue();

    void incFrameAndUpdateParentAnimator(int inc);

    void incValue(qreal incBy);

    CtrlsMode getCtrlsMode();
    int saveToSql(int parentAnimatorSqlId);
    void loadFromSql(int keyId);

    virtual void copyToContainer(KeysClipboardContainer *container);

    QrealAnimator *getParentQrealAnimator();
    void setRelFrame(int frame);

    bool differsFromKey(Key *key);

    KeyCloner *createNewKeyCloner();

protected:
    CtrlsMode mCtrlsMode = CTRLS_SYMMETRIC;

    QrealPoint *mGraphPoint;
    QrealPoint *mStartPoint;
    QrealPoint *mEndPoint;

    qreal mValue;
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

class QrealKeyCloner : public KeyCloner {
public:
    QrealKeyCloner(QrealKey *key);

    Key *createKeyForAnimator(Animator *parentAnimator);
private:
    qreal mValue;
};

#endif // QREALKEY_H
