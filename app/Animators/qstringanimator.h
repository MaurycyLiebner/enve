#ifndef QSTRINGANIMATOR_H
#define QSTRINGANIMATOR_H
#include "Animators/animator.h"
#include "Animators/key.h"
class QStringAnimator;

class QStringKey : public Key {
    friend class StdSelfRef;
public:

    bool differsFromKey(Key *key) const;

    const QString &getText() { return mText; }
    void setText(const QString &text) { mText = text; }
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);
protected:
    QStringKey(const QString &stringT,
               const int &relFrame,
               QStringAnimator *parentAnimator = nullptr);
private:
    QString mText;
};

class QStringAnimator : public Animator {
    friend class SelfRef;
public:
    void prp_setAbsFrame(const int &frame);
    void setCurrentTextValue(const QString &text);
    QString getCurrentTextValue();

    bool SWT_isQStringAnimator() const { return true; }
    QString getTextValueAtRelFrame(const int &relFrame);
    void anim_saveCurrentValueAsKey();
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
protected:
    QStringAnimator(const QString& name);
private:
    QString mCurrentText;
};

#endif // QSTRINGANIMATOR_H
