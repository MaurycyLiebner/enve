#ifndef QSTRINGANIMATOR_H
#define QSTRINGANIMATOR_H
#include "animator.h"
#include "key.h"
class QStringAnimator;

class QStringKey : public Key {
public:
    QStringKey(const QString &stringT,
               const int &relFrame,
               QStringAnimator *parentAnimator = NULL);

    bool differsFromKey(Key *key);

    const QString &getText() { return mText; }
    void setText(const QString &text) { mText = text; }
    QStringKey *makeDuplicate(QStringAnimator *anim);
    void writeQStringKey(QIODevice *target);
    void readQStringKey(QIODevice *target);
private:
    QString mText;
};

class QStringAnimator : public Animator
{
public:
    QStringAnimator();

    void prp_setAbsFrame(const int &frame);
    void setCurrentTextValue(const QString &text,
                             const bool &saveUndoRedo = true);
    QString getCurrentTextValue();

    bool SWT_isQStringAnimator() { return true; }
    QString getTextValueAtRelFrame(const int &relFrame);
    void anim_saveCurrentValueAsKey();
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                              const int &relFrame);

    void makeDuplicate(QStringAnimator *anim);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:

    QString mCurrentText;
    QList<QStringKey*> mTextValues;
};

#endif // QSTRINGANIMATOR_H
