#ifndef VALUEINPUT_H
#define VALUEINPUT_H
#include <QString>
#include <QPointF>
#include <QLocale>
class QPainter;
class SkCanvas;
class QKeyEvent;

enum TransformMode : char {
    MODE_MOVE, MODE_SCALE, MODE_ROTATE, MODE_NONE
};

class ValueInput {
public:
    ValueInput();

    void draw(SkCanvas *canvas, const int y);
    void draw(QPainter *p, const int y);

    qreal getValue() const { return mInputValue; }
    QPointF getPtValue() const {
        if(mTransMode == MODE_ROTATE ||
               mXYMode == MODE_XY)
            return {mInputValue, mInputValue};
        if(mTransMode == MODE_SCALE) {
            if(mXYMode == MODE_X) return {mInputValue, 1};
            else return {1, mInputValue};
        } else { //if(mTransMode == MODE_MOVE) {
            if(mXYMode == MODE_X) return {mInputValue, 0};
            else return {0, mInputValue};
        }
    }

    void clearAndDisableInput();

    bool handleTransormationInputKeyEvent(QKeyEvent *event);

    bool inputEnabled() const { return mInputEnabled; }

    void setupRotate() {
        mTransMode = MODE_ROTATE;
        setDisplayedValue(0);
    }

    void setupScale() {
        mTransMode = MODE_SCALE;
        setDisplayedValue(1);
    }

    void setupMove() {
        mTransMode = MODE_MOVE;
        setDisplayedValue(0);
    }

    void setXOnlyMode() {
        mXYMode = MODE_X;
    }

    void setYOnlyMode() {
        mXYMode = MODE_Y;
    }

    void setXYMode() {
        mXYMode = MODE_XY;
    }

    void switchXOnlyMode() {
        if(mXYMode == MODE_X) setXYMode();
        else setXOnlyMode();
    }

    void switchYOnlyMode() {
        if(mXYMode == MODE_Y) setXYMode();
        else setYOnlyMode();
    }

    bool xOnlyMode() const {
        return mXYMode == MODE_X;
    }

    bool yOnlyMode() const {
        return mXYMode == MODE_Y;
    }

    void setDisplayedValue(const qreal value) {
        setDisplayedValue({value, value});
    }

    void setDisplayedValue(const QPointF& value) {
        mDisplayValue = value;
    }

    void setForce1D(const bool d) {
        mForce1D = d;
    }
protected:
    void updateInputValue();

    QString getText() const {
        QString transStr;
        if(mInputEnabled) {
            transStr = getNameWithXY() + ": " + mInputText + "|";
        } else if(mTransMode == MODE_ROTATE) {
            const auto xVal = QLocale().toString(mDisplayValue.x(), 'f', 3);
            transStr = getName() + ": " + xVal;
        } else if(xOnlyMode()) {
            const auto xVal = QLocale().toString(mDisplayValue.x(), 'f', 3);
            transStr = getNameWithXY() + ": " + xVal;
        } else if(yOnlyMode()) {
            const auto yVal = QLocale().toString(mDisplayValue.y(), 'f', 3);
            transStr = getNameWithXY() + ": " + yVal;
        } else { // xy mode
            const auto xVal = QLocale().toString(mDisplayValue.x(), 'f', 3);
            const auto yVal = QLocale().toString(mDisplayValue.y(), 'f', 3);
            transStr = getName() + " x, y: " + xVal + ", " + yVal;
        }
        return transStr;
    }

    QString getName() const {
        if(mTransMode == MODE_MOVE) {
            return "move";
        } else if(mTransMode == MODE_SCALE) {
            return "scale";
        } else { // if(mTransMode == MODE_ROTATE) {
            return "rotate";
        }
    }

    QString getNameWithXY() const {
        if(mForce1D) return getName();
        if(mTransMode == MODE_MOVE) {
            if(mXYMode == MODE_XY) return "move x, y";
            else if(mXYMode == MODE_X) return "move x";
            else /*if(mXYMode == MODE_Y)*/ return "move y";
        } else if(mTransMode == MODE_SCALE) {
            if(mXYMode == MODE_XY) return "scale x, y";
            else if(mXYMode == MODE_X) return "scale x";
            else /*if(mXYMode == MODE_Y)*/ return "scale y";
        } else { // if(mTransMode == MODE_ROTATE) {
            return "rotate";
        }
    }

    TransformMode mTransMode = MODE_MOVE;
    enum { MODE_X, MODE_Y, MODE_XY } mXYMode = MODE_XY;
    QString mInputText;
    qreal mInputValue = 0;
    QPointF mDisplayValue;
    bool mInputEnabled = false;
    bool mForce1D = false;
};

#endif // VALUEINPUT_H
