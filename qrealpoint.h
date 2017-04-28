#ifndef QREALPOINT_H
#define QREALPOINT_H
#include <QPainter>
#include "selfref.h"

class QrealKey;

enum QrealPointType : short {
    START_POINT,
    END_POINT,
    KEY_POINT
};

class QrealPoint : public StdSelfRef
{
public:
    QrealPoint(QrealPointType type, QrealKey *parentKey, qreal radius = 10.);
    ~QrealPoint() {}

    qreal getFrame();

    void setFrame(qreal frame);

    virtual qreal getValue();

    virtual void setValue(qreal value, bool finish = false);

    bool isSelected();

    bool isNear(qreal frameT, qreal valueT,
                qreal pixelsPerFrame, qreal pixelsPerValue);

    void moveTo(qreal frameT, qreal valueT);

    void draw(QPainter *p, const QColor &paintColor,
              const qreal &minFrameT, const qreal &minValueT,
              const qreal &pixelsPerFrame, const qreal &pixelsPerValue);

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
