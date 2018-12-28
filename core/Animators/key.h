#ifndef KEY_H
#define KEY_H
#include "smartPointers/sharedpointerdefs.h"
#include "pointhelpers.h"

#include <QtCore>
class QPainter;

class ComplexAnimator;
class QrealPoint;

class ComplexKey;
class KeyCloner;

class Animator;
class KeysClipboardContainer;
enum QrealPointType : short;

class Key : public StdSelfRef {
public:
    Key(Animator * const parentAnimator);
//    QrealPoint *mousePress(qreal frameT, qreal valueT,
//                    qreal pixelsPerFrame, qreal pixelsPerValue);
    virtual ~Key() {}

    virtual void startFrameTransform();
    virtual void finishFrameTransform();

    int getAbsFrame() const;
    virtual void setRelFrame(const int &frame);

    bool hasPrevKey() const;
    bool hasNextKey() const;
    void incFrameAndUpdateParentAnimator(const int &inc,
                                         const bool &finish = true);
    void setRelFrameAndUpdateParentAnimator(const int &relFrame,
                                         const bool &finish = true);
    template <class T = Animator>
    T* getParentAnimator() const {
        return static_cast<T*>(mParentAnimator.data());
    }

    virtual void mergeWith(const stdsptr<Key>& key) { key->removeFromAnimator(); }

    virtual bool isDescendantSelected() const { return isSelected(); }

    void removeFromAnimator();

    virtual void deleteKey() {
        removeFromAnimator();
    }

    virtual void cancelFrameTransform();
    virtual void scaleFrameAndUpdateParentAnimator(
            const int &relativeToFrame,
            const qreal &scaleFactor,
            const bool &useSavedFrame = true);
    void setSelected(const bool &bT);
    virtual bool isSelected() const;

    virtual void addToSelection(QList<qptr<Animator>> &selectedAnimators);
    virtual void removeFromSelection(QList<qptr<Animator>> &selectedAnimators);

    bool isHovered() const {
        return mHovered;
    }

    void setHovered(const bool &bT) {
        mHovered = bT;
    }

    int getRelFrame() const;
    void setAbsFrame(const int &frame);

    Key* getNextKey() const;
    Key *getPrevKey() const;

    bool differesFromNextKey() const {
        return differsFromKey(getNextKey());
    }

    bool differesFromPrevKey() const {
        return differsFromKey(getPrevKey());
    }

    virtual bool differsFromKey(Key* key) const = 0;
    virtual void writeKey(QIODevice *target);
    virtual void readKey(QIODevice *target);

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
    void afterKeyChanged();
    void setCtrlsMode(const CtrlsMode &mode);
    const CtrlsMode& getCtrlsMode() const;

    void constrainStartCtrlMinFrame(const int &minFrame);
    void constrainEndCtrlMaxFrame(const int &maxFrame);
    virtual void changeFrameAndValueBy(const QPointF &frameValueChange);
    bool isInsideRect(const QRectF &valueFrameRect) const;
    virtual void saveCurrentFrameAndValue() {
        mSavedRelFrame = mRelFrame;
    }

    virtual qreal getPrevKeyValueForGraph() const {
        auto prevKey = getPrevKey();
        if(!prevKey) return getValueForGraph();
        return prevKey->getValueForGraph();
    }
    virtual qreal getNextKeyValueForGraph() const {
        auto nextKey = getNextKey();
        if(!nextKey) return getValueForGraph();
        return nextKey->getValueForGraph();
    }

    int getPrevKeyRelFrame() const {
        auto prevKey = getPrevKey();
        if(!prevKey) return mRelFrame;
        return prevKey->getRelFrame();
    }
    int getNextKeyRelFrame() const {
        auto nextKey = getNextKey();
        if(!nextKey) return mRelFrame;
        return getNextKey()->getRelFrame();
    }
    void makeStartAndEndSmooth();
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

    bool mIsSelected = false;
    bool mHovered = false;

    int mRelFrame;
    int mSavedRelFrame;

    QPointer<Animator> mParentAnimator;

    stdsptr<QrealPoint> mGraphPoint;
    stdsptr<QrealPoint> mStartPoint;
    stdsptr<QrealPoint> mEndPoint;

    CtrlsMode mCtrlsMode = CTRLS_SYMMETRIC;
};

struct KeyPair {
    KeyPair(Key* key1T, Key* key2T) {
        key1 = key1T;
        key2 = key2T;
    }

    void merge() const {
        key1->mergeWith(GetAsSPtr(key2, Key));
    }

    Key* key1;
    Key* key2;
};

#endif // KEY_H
