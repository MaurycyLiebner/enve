#ifndef VALUEINPUT_H
#define VALUEINPUT_H
#include <QString>
#include <QPointF>
#include <QLocale>
class QPainter;
class SkCanvas;
class QKeyEvent;

enum class TransformMode { move, scale, rotate, none };
enum class DirectionMode { x, y, xy };

class ValueInput {
public:
    ValueInput();

    void draw(SkCanvas *canvas, const int y);
    void draw(QPainter *p, const int y);

    qreal getValue() const { return mInputValue; }
    QPointF getPtValue() const {
        if(mTransMode == TransformMode::rotate ||
               mXYMode == DirectionMode::xy)
            return {mInputValue, mInputValue};
        if(mTransMode == TransformMode::scale) {
            if(mXYMode == DirectionMode::x) return {mInputValue, 1};
            else return {1, mInputValue};
        } else { //if(mTransMode == MODE_MOVE) {
            if(mXYMode == DirectionMode::x) return {mInputValue, 0};
            else return {0, mInputValue};
        }
    }

    void clearAndDisableInput();

    bool handleTransormationInputKeyEvent(const int key);

    bool inputEnabled() const { return mInputEnabled; }

    void setupRotate() {
        mTransMode = TransformMode::rotate;
        setDisplayedValue(0);
    }

    void setupScale() {
        mTransMode = TransformMode::scale;
        setDisplayedValue(1);
    }

    void setupMove() {
        mTransMode = TransformMode::move;
        setDisplayedValue(0);
    }

    void setXOnlyMode() {
        mXYMode = DirectionMode::x;
    }

    void setYOnlyMode() {
        mXYMode = DirectionMode::y;
    }

    void setXYMode() {
        mXYMode = DirectionMode::xy;
    }

    void switchXOnlyMode() {
        if(mXYMode == DirectionMode::x) setXYMode();
        else setXOnlyMode();
    }

    void switchYOnlyMode() {
        if(mXYMode == DirectionMode::y) setXYMode();
        else setYOnlyMode();
    }

    bool xOnlyMode() const {
        return mXYMode == DirectionMode::x;
    }

    bool yOnlyMode() const {
        return mXYMode == DirectionMode::y;
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
        } else if(mTransMode == TransformMode::rotate) {
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
        if(mTransMode == TransformMode::move) {
            return "move";
        } else if(mTransMode == TransformMode::scale) {
            return "scale";
        } else { // if(mTransMode == MODE_ROTATE) {
            return "rotate";
        }
    }

    QString getNameWithXY() const {
        if(mForce1D) return getName();
        if(mTransMode == TransformMode::move) {
            if(mXYMode == DirectionMode::xy) return "move x, y";
            else if(mXYMode == DirectionMode::x) return "move x";
            else /*if(mXYMode == MODE_Y)*/ return "move y";
        } else if(mTransMode == TransformMode::scale) {
            if(mXYMode == DirectionMode::xy) return "scale x, y";
            else if(mXYMode == DirectionMode::x) return "scale x";
            else /*if(mXYMode == MODE_Y)*/ return "scale y";
        } else { // if(mTransMode == MODE_ROTATE) {
            return "rotate";
        }
    }

    TransformMode mTransMode = TransformMode::move;
    DirectionMode mXYMode = DirectionMode::xy;
    QString mInputText;
    qreal mInputValue = 0;
    QPointF mDisplayValue;
    bool mInputEnabled = false;
    bool mForce1D = false;
};

#endif // VALUEINPUT_H
