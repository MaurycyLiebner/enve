#ifndef QREALPOINT_H
#define QREALPOINT_H
#include <QPainter>
#include "../smartPointers/sharedpointerdefs.h"

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

    qreal getRelFrame();
    qreal getAbsFrame();

    void setRelFrame(const qreal frame);
    void setAbsFrame(const qreal absFrame);

    qreal getValue();

    void setValue(const qreal value);

    bool isSelected();

    bool isNear(const qreal frameT,
                const qreal valueT,
                const qreal pixelsPerFrame,
                const qreal pixelsPerValue);

    void moveTo(const qreal frameT, const qreal valueT);

    void draw(QPainter * const p, const QColor &paintColor);

    void setSelected(const bool bT);

    bool isKeyPoint();
    bool isStartPoint();
    bool isEndPoint();

    bool isEnabled();

    GraphKey *getParentKey();
    void startFrameTransform();
    void finishFrameTransform();
    const QrealPointType& getType() const {
        return mType;
    }

    void setHovered(const bool hovered) {
        mHovered = hovered;
    }
private:
    QrealPoint(const QrealPointType &type,
               GraphKey * const parentKey,
               const qreal radius = 10);
    bool mIsSelected = false;
    bool mHovered = false;
    QrealPointType mType;
    GraphKey *mParentKey;
    qreal mRadius;
};


#endif // QREALPOINT_H
