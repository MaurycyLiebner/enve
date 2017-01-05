#ifndef QSTRINGANIMATOR_H
#define QSTRINGANIMATOR_H
#include "intanimator.h"

struct QStringKey {
    QStringKey(QString stringT, int frame) {
        string = stringT;
        keyFrame = frame;
    }

    QString string;
    int keyFrame;
};

class QStringAnimator : public IntAnimator
{
public:
    QStringAnimator();

    void setCurrentTextValue(const QString &text);
    QString getCurrentTextValue();

    QString getStringKeyWithLowerFrame(int frame);
private:
    QList<QStringKey*> mTextValues;
};

#endif // QSTRINGANIMATOR_H
