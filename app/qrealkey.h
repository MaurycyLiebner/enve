#ifndef QREALKEY_H
#define QREALKEY_H
#include "Animators/key.h"
#include "smartPointers/sharedpointerdefs.h"

class QPainter;
class ComplexAnimator;

class ComplexKey;

class QrealAnimator;
class KeysClipboardContainer;

class QrealKey : public Key {
    friend class StdSelfRef;
public:
    QrealKey(QrealAnimator* parentAnimator);
    QrealKey(const int &frame,
             const qreal &val,
             QrealAnimator* parentAnimator);

    stdsptr<QrealKey> makeQrealKeyDuplicate(QrealAnimator *targetParent);


    qreal getValue() const;
    virtual void setValue(qreal value,
                          const bool &saveUndoRedo = false,
                          const bool &finish = false,
                          const bool &callUpdater = true);
    void setStartValueVar(const qreal &value);
    void setEndValueVar(const qreal &value);

    void setStartFrameVar(const qreal &startFrame);
    void setEndFrameVar(const qreal &endFrame);

    qreal getStartValue() const;
    qreal getEndValue() const;
    qreal getStartValueFrame() const;
    qreal getEndValueFrame() const;

    void setStartEnabledForGraph(const bool &bT);
    void setEndEnabledForGraph(const bool &bT);

    void changeFrameAndValueBy(const QPointF &frameValueChange);
    void saveCurrentFrameAndValue();

    bool getEndEnabledForGraph() const;
    bool getStartEnabledForGraph() const;

    qreal getPrevKeyValueForGraph() const;
    qreal getNextKeyValueForGraph() const;

    void incValue(const qreal &incBy,
                  const bool &saveUndoRedo = false,
                  const bool &finish = false,
                  const bool &callUpdater = true);

    QrealAnimator *getParentQrealAnimator() const;
    void setRelFrame(const int &frame);

    bool differsFromKey(Key *key) const;

    void scaleFrameAndUpdateParentAnimator(
            const int &relativeToFrame,
            const qreal &scaleFactor,
            const bool &useSavedFrame = true);

    void startValueTransform();
    void finishValueTransform();
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);
    void startFrameTransform() {
        Key::startFrameTransform();
        mSavedStartFrame = mStartFrame;
        mSavedEndFrame = mEndFrame;
    }

    qreal getValueForGraph() const {
        return getValue();
    }

    qreal getEndValueForGraph() const {
        return getEndValue();
    }

    qreal getStartValueForGraph() const {
        return getStartValue();
    }

    qreal getStartFrameForGraph() const {
        return getStartValueFrame();
    }

    qreal getEndFrameForGraph() const {
        return getEndValueFrame();
    }

    void setEndFrameForGraph(const qreal &endFrame);
    void setStartFrameForGraph(const qreal &startFrame);
    void setStartValueForGraph(const qreal &value);
    void setEndValueForGraph(const qreal &value);

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

protected:
    bool mStartEnabled = false;
    bool mEndEnabled = false;

    qreal mValue;
    qreal mSavedValue;

    qreal mSavedStartFrame;
    qreal mSavedEndFrame;

    qreal mStartValue = 0.;
    qreal mEndValue = 0.;
    qreal mStartFrame = 0.;
    qreal mEndFrame = 0.;
};

#endif // QREALKEY_H
