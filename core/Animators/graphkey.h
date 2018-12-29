#ifndef GRAPHKEY_H
#define GRAPHKEY_H
#include "key.h"
enum QrealPointType : short;
class QrealPoint;

class GraphKey : public Key {
public:
    GraphKey(const int &frame,
             Animator * const parentAnimator);
    GraphKey(Animator * const parentAnimator);

    virtual void setValueForGraph(const qreal& value) = 0;
    virtual qreal getValueForGraph() const = 0;

    qreal getEndValue() const;
    qreal getStartValue() const;

    void setEndValueForGraph(const qreal& value);
    void setStartValueForGraph(const qreal& value);

    void drawGraphKey(QPainter * const p,
                      const QColor &paintColor) const;

    qreal getEndValueDirectionForGraph() const {
        return getEndValueDirectionForGraphForEndValue(
                    getEndValue());
    }

    qreal getStartValueDirectionForGraph() const {
        return getStartValueDirectionForGraphForStartValue(
                    getStartValue());
    }

    void setEndValueDirectionForGraph(const qreal &value) {
        if(!hasNextKey()) return;
        setEndValueForGraph(
                    getEndValueForGraphEndValueDirection(value));
    }

    void setStartValueDirectionForGraph(const qreal &value) {
        if(!hasPrevKey()) return;
        setStartValueForGraph(
                    getStartValueForGraphStartValueDirection(value));
    }

    void setStartFrame(const qreal& frame);
    void setEndFrame(const qreal& frame);

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

    qreal getPrevKeyValueForGraph() const {
        auto prevKey = getPrevKey<GraphKey>();
        if(!prevKey) return getValueForGraph();
        return prevKey->getValueForGraph();
    }
    qreal getNextKeyValueForGraph() const {
        auto nextKey = getNextKey<GraphKey>();
        if(!nextKey) return getValueForGraph();
        return nextKey->getValueForGraph();
    }

    void makeStartAndEndSmooth();
    bool isInsideRect(const QRectF &valueFrameRect) const;

    void setStartFrameVar(const qreal &startFrame);
    void setEndFrameVar(const qreal &endFrame);

    qreal getStartFrame() const;
    qreal getEndFrame() const;

    void setStartEnabledForGraph(const bool &bT);
    void setEndEnabledForGraph(const bool &bT);

    bool getEndEnabledForGraph() const;
    bool getStartEnabledForGraph() const;

    virtual void saveCurrentFrameAndValue() {
        mSavedRelFrame = mRelFrame;
    }

    void startFrameTransform() {
        Key::startFrameTransform();
        mSavedStartFrame = mStartFrame;
        mSavedEndFrame = mEndFrame;
    }

    void scaleFrameAndUpdateParentAnimator(
            const int &relativeToFrame,
            const qreal &scaleFactor,
            const bool& useSavedFrame);
    void setRelFrame(const int &frame);
    void setStartValueVar(const qreal &value);
    void setEndValueVar(const qreal &value);
    void constrainEndCtrlValue(const qreal &minVal,
                               const qreal &maxVal);
    void constrainStartCtrlValue(const qreal &minVal,
                                 const qreal &maxVal);
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

    bool mStartEnabled = false;
    bool mEndEnabled = false;

    qreal mSavedStartFrame;
    qreal mSavedEndFrame;
    qreal mStartFrame = 0.;
    qreal mEndFrame = 0.;

    qreal mStartValue = 0.;
    qreal mEndValue = 0.;

    stdsptr<QrealPoint> mGraphPoint;
    stdsptr<QrealPoint> mStartPoint;
    stdsptr<QrealPoint> mEndPoint;

    CtrlsMode mCtrlsMode = CTRLS_SYMMETRIC;
};

#endif // GRAPHKEY_H
