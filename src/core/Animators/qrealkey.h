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
    QrealKey(QrealAnimator * const parentAnimator);
    QrealKey(const qreal value,
             const int frame,
             QrealAnimator * const parentAnimator);

    qreal getValue() const;
    void setValue(const qreal value);

    void changeFrameAndValueBy(const QPointF &frameValueChange);
    void incValue(const qreal incBy);

    QrealAnimator *getParentQrealAnimator() const;

    bool differsFromKey(Key *key) const;

    void startValueTransform();
    void finishValueTransform();
    void cancelValueTransform();
    void writeKey(QIODevice * const dst);
    void readKey(QIODevice * const src);

    qreal getValueForGraph() const {
        return getValue();
    }

    void setValueForGraph(const qreal value) {
        setValue(value);
    }
protected:
    qreal mValue;
    qreal mSavedValue;
};

#endif // QREALKEY_H
