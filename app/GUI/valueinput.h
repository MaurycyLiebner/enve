#ifndef VALUEINPUT_H
#define VALUEINPUT_H
#include <QString>
class QPainter;
class SkCanvas;
class QKeyEvent;
class ValueInput {
public:
    ValueInput();

    void draw(SkCanvas *canvas, const int &y);
    void draw(QPainter *p, const int &y);

    const qreal &getValue() { return mInputTransformationValue; }
    void clearAndDisableInput();
    void updateInputValue();
    bool handleTransormationInputKeyEvent(QKeyEvent *event);
    void setName(const QString &str) {
        mName = str;
    }

    const bool &inputEnabled() { return mInputTransformationEnabled; }
protected:
    QString mInputText;
    qreal mInputTransformationValue = 0.;
    bool mInputTransformationEnabled = false;
    QString mName;
};

#endif // VALUEINPUT_H
