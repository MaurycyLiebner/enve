#ifndef QREALKEY_H
#define QREALKEY_H
#include "Animators/graphkey.h"
#include "smartPointers/sharedpointerdefs.h"

class QPainter;
class ComplexAnimator;

class ComplexKey;

class QrealAnimator;
class KeysClipboardContainer;

class QrealKey : public GraphKey {
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

    void changeFrameAndValueBy(const QPointF &frameValueChange);
    void saveCurrentFrameAndValue();

    void incValue(const qreal &incBy,
                  const bool &saveUndoRedo = false,
                  const bool &finish = false,
                  const bool &callUpdater = true);

    QrealAnimator *getParentQrealAnimator() const;

    bool differsFromKey(Key *key) const;

    void startValueTransform();
    void finishValueTransform();
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);

    qreal getValueForGraph() const {
        return getValue();
    }

    qreal getStartValue() const;
    qreal getEndValue() const;

    void setStartValueForGraph(const qreal &value);
    void setEndValueForGraph(const qreal &value);
protected:
    qreal mValue;
    qreal mSavedValue;

    qreal mStartValue = 0.;
    qreal mEndValue = 0.;
};

#endif // QREALKEY_H
