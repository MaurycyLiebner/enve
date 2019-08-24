#ifndef ENVESPLASH_H
#define ENVESPLASH_H
#include <QSplashScreen>

class EnveSplash : public QSplashScreen {
public:
    EnveSplash();
protected:
    void drawContents(QPainter* const p);
    void mousePressEvent(QMouseEvent *);
private:
    QString mText;
    QRect mTextRect;
    QRect mMessageRect;
};

#endif // ENVESPLASH_H
