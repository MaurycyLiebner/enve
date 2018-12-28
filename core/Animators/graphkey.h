#ifndef GRAPHKEY_H
#define GRAPHKEY_H
#include "key.h"
enum QrealPointType : short;
class QrealPoint;

class GraphKey : public Key {
public:
    GraphKey(Animator * const parentAnimator);

    void drawGraphKey(QPainter * const p,
                      const QColor &paintColor) const;

    virtual qreal getValueForGraph() const {
        return mRelFrame;
    }

    virtual qreal getEndValueForGraph() const {
        return mRelFrame;
    }

    virtual qreal getStartValueForGraph() const {
        return mRelFrame;
    }

    qreal getEndValueDirectionForGraph() const {
        return getEndValueDirectionForGraphForEndValue(
                    getEndValueForGraph());
    }

    qreal getStartValueDirectionForGraph() const {
        return getStartValueDirectionForGraphForStartValue(
                    getStartValueForGraph());
    }

    virtual qreal getStartFrameForGraph() const {
        return mRelFrame;
    }

    virtual qreal getEndFrameForGraph() const {
        return mRelFrame;
    }

    virtual bool getStartEnabledForGraph() const {
        return false;
    }

    virtual bool getEndEnabledForGraph() const {
        return false;
    }

    virtual void setValueForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    virtual void setEndValueForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    virtual void setStartValueForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    virtual void setEndValueDirectionForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    virtual void setStartValueDirectionForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    virtual void setStartFrameForGraph(const qreal& frame) {
        Q_UNUSED(frame);
    }

    virtual void setEndFrameForGraph(const qreal& frame) {
        Q_UNUSED(frame);
    }

    virtual void setStartEnabledForGraph(const bool& enabled) {
        Q_UNUSED(enabled);
    }

    virtual void setEndEnabledForGraph(const bool& enabled) {
        Q_UNUSED(enabled);
    }

    QrealPoint *mousePress(const qreal &frameT,
                           const qreal &valueT,
                           const qreal &pixelsPerFrame,
                           const qreal &pixelsPerValue);

    void updateCtrlFromCtrl(const QrealPointType &type);
    void setCtrlsMode(const CtrlsMode &mode);
    const CtrlsMode& getCtrlsMode() const;

    void constrainStartCtrlMinFrame(const int &minFrame);
    void constrainEndCtrlMaxFrame(const int &maxFrame);
    virtual void changeFrameAndValueBy(const QPointF &frameValueChange);

    virtual qreal getPrevKeyValueForGraph() const {
        auto prevKey = getPrevKey<GraphKey>();
        if(!prevKey) return getValueForGraph();
        return prevKey->getValueForGraph();
    }
    virtual qreal getNextKeyValueForGraph() const {
        auto nextKey = getNextKey<GraphKey>();
        if(!nextKey) return getValueForGraph();
        return nextKey->getValueForGraph();
    }

    void makeStartAndEndSmooth();
    bool isInsideRect(const QRectF &valueFrameRect) const;
protected:
    qreal getEndValueDirectionForGraphForEndValue(const qreal& endVal) const {
        if(!hasNextKey()) return 0.;
        qreal nextValue = getNextKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = getNextKeyRelFrame() - mRelFrame;
        return (endVal - valG)/(nextValue - valG)/dFrame;
    }

    qreal getStartValueDirectionForGraphForStartValue(const qreal& startVal) const {
        if(!hasPrevKey()) return 0.;
        qreal prevValue = getPrevKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = mRelFrame - getPrevKeyRelFrame();
        return (startVal - valG)/(prevValue - valG)/dFrame;
    }

    qreal getStartValueForGraphStartValueDirection(const qreal& value) const {
        if(!hasPrevKey()) return 0.;
        qreal prevValue = getPrevKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = mRelFrame - getPrevKeyRelFrame();
        return valG + (prevValue - valG)*value*dFrame;
    }

    qreal getEndValueForGraphEndValueDirection(const qreal& value) const {
        if(!hasNextKey()) return 0.;
        qreal nextValue = getNextKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = getNextKeyRelFrame() - mRelFrame;
        return valG + (nextValue - valG)*value*dFrame;
    }

    stdsptr<QrealPoint> mGraphPoint;
    stdsptr<QrealPoint> mStartPoint;
    stdsptr<QrealPoint> mEndPoint;

    CtrlsMode mCtrlsMode = CTRLS_SYMMETRIC;
};

#endif // GRAPHKEY_H
