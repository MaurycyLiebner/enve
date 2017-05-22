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
    QrealPoint *mousePress(const qreal &frameT,
                           const qreal &valueT,
                           const qreal &pixelsPerFrame,
                           const qreal &pixelsPerValue);
    virtual ~QrealKey();


    QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    void updateCtrlFromCtrl(const QrealPointType &type);

    virtual qreal getValue();
    virtual void setValue(qreal value,
                          const bool &saveUndoRedo = true);
    void setStartValueVar(const qreal &value);
    void setEndValueVar(const qreal &value);

    void setStartFrameVar(const qreal &startFrame);
    void setEndFrameVar(const qreal &endFrame);

    qreal getStartValue();
    qreal getEndValue();
    qreal getStartValueFrame();
    qreal getEndValueFrame();

    virtual void setStartEnabled(const bool &bT);

    virtual void setEndEnabled(const bool &bT);

    bool isInsideRect(const QRectF &valueFrameRect);

    void drawGraphKey(QPainter *p, const QColor &paintColor);
    void changeFrameAndValueBy(const QPointF &frameValueChange);
    void saveCurrentFrameAndValue();
    virtual void setCtrlsMode(const CtrlsMode &mode);

    void constrainStartCtrlMinFrame(const int &minFrame);
    void constrainEndCtrlMaxFrame(const int &maxFrame);

    QrealPoint *getStartPoint();
    QrealPoint *getEndPoint();
    QrealPoint *getGraphPoint();

    bool isEndPointEnabled();
    bool isStartPointEnabled();

    qreal getPrevKeyValue();
    qreal getNextKeyValue();

    void incValue(qreal incBy);

    CtrlsMode getCtrlsMode();
    int saveToSql(const int &parentAnimatorSqlId);
    void loadFromSql(const int &keyId);

    virtual void copyToContainer(KeysClipboardContainer *container);

    QrealAnimator *getParentQrealAnimator();
    void setRelFrame(const int &frame);

    bool differsFromKey(Key *key);

    KeyCloner *createNewKeyCloner();

    void setEndFrame(const qreal &endFrame);
    void setStartFrame(const qreal &startFrame);
    void setStartValue(const qreal &value);
    void setEndValue(const qreal &value);

    void startValueTransform();
    void finishValueTransform();
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
