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
    int saveToSql(const int &parentAnimatorSqlId);
    void loadFromSql(const int &keyId);

    static QStringKey *qStringKeyFromSql(const int &keyId);
private:
    QString mText;
};

class QStringAnimator : public Animator
{
public:
    QStringAnimator();

    void prp_setAbsFrame(const int &frame);
    void setCurrentTextValue(const QString &text);
    QString getCurrentTextValue();

    bool SWT_isQStringAnimator() { return true; }
    QString getTextValueAtRelFrame(const int &relFrame);
    void anim_saveCurrentValueAsKey();
    void loadKeysFromSql(const int &qrealAnimatorId);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                              const int &relFrame);
    void loadFromSql(const int &qstringAnimatorId);
    int saveToSql(QSqlQuery *query, const int &parentId);
public slots:
    void prp_setRecording(const bool &rec);
private:

    QString mCurrentText;
    QList<QStringKey*> mTextValues;
};

#endif // QSTRINGANIMATOR_H
