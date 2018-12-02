#ifndef QREALPOINT_H
#define QREALPOINT_H
#include <QPainter>
#include "smartPointers/sharedpointerdefs.h"

class QrealKey;

enum QrealPointType : short {
    START_POINT,
    END_POINT,
    KEY_POINT
};

class QrealPoint : public StdSelfRef {
public:
    QrealPoint(QrealPointType type,
               QrealKey *parentKey,
               const qreal &radius = 10.);
    ~QrealPoint() {}

    qreal getFrame();

    void setFrame(const qreal &frame);

    virtual qreal getValue();

    virtual void setValue(const qreal &value);

    bool isSelected();

    bool isNear(const qreal &frameT,
                const qreal &valueT,
                const qreal &pixelsPerFrame,
                const qreal &pixelsPerValue);

    void moveTo(const qreal &frameT, const qreal &valueT);

    void draw(QPainter *p, const QColor &paintColor);

    void setSelected(const bool &bT);

    bool isKeyPoint();
    bool isStartPoint();
    bool isEndPoint();

    bool isEnabled();

    QrealKey *getParentKey();
    void startFrameTransform();
    void finishFrameTransform();
private:
    bool mIsSelected = false;
    QrealPointType mType;
    QrealKey *mParentKey;
    qreal mRadius;
};


#endif // QREALPOINT_H
