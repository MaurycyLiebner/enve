#ifndef QREALPOINT_H
#define QREALPOINT_H
#include <QPainter>
#include "smartpointertarget.h"

class QrealKey;

enum QrealPointType : short {
    START_POINT,
    END_POINT,
    KEY_POINT
};

class QrealPoint : public SmartPointerTarget
{
public:
    QrealPoint(QrealPointType type, QrealKey *parentKey, qreal radius = 10.);

    qreal getFrame();

    void setFrame(qreal frame);

    virtual qreal getValue();

    virtual void setValue(qreal value, bool finish = false);

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

    bool isEnabled();

    QrealKey *getParentKey();
private:
    bool mIsSelected = false;
    QrealPointType mType;
    QrealKey *mParentKey;
    qreal mRadius;
};


#endif // QREALPOINT_H
