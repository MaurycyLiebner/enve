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


    qreal getValue() const;
    void setValue(const qreal &value);

    void changeFrameAndValueBy(const QPointF &frameValueChange);
    void saveCurrentFrameAndValue();

    void incValue(const qreal &incBy);

    QrealAnimator *getParentQrealAnimator() const;

    bool differsFromKey(Key *key) const;

    void startValueTransform();
    void finishValueTransform();
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);

    qreal getValueForGraph() const {
        return getValue();
    }

    void setValueForGraph(const qreal& value) {
        setValue(value);
    }
protected:
    qreal mValue;
    qreal mSavedValue;
};

#endif // QREALKEY_H
