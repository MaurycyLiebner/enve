#ifndef QREALPOINT_H
#define QREALPOINT_H
#include <QPainter>
#include "smartPointers/sharedpointerdefs.h"

class GraphKey;

enum QrealPointType : short {
    START_POINT,
    END_POINT,
    KEY_POINT
};

class QrealPoint : public StdSelfRef {
    friend class StdSelfRef;
public:
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

    GraphKey *getParentKey();
    void startFrameTransform();
    void finishFrameTransform();
private:
    QrealPoint(const QrealPointType &type,
               GraphKey * const parentKey,
               const qreal &radius = 10.);
    bool mIsSelected = false;
    QrealPointType mType;
    GraphKey *mParentKey;
    qreal mRadius;
};


#endif // QREALPOINT_H
