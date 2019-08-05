#include "valueinput.h"

#include <QPainter>
#include <QApplication>

#include "skia/skiaincludes.h"
#include "skia/skqtconversions.h"
#include "GUI/global.h"

ValueInput::ValueInput() {

}

void ValueInput::draw(SkCanvas *canvas, const int y) {
    SkPaint paint;

    const auto transStr = getText();
    const int textWidth = QApplication::fontMetrics().width(transStr);
    const SkRect inputRect =
            SkRect::MakeXYWH(2*MIN_WIDGET_DIM, y,
                             textWidth + MIN_WIDGET_DIM, MIN_WIDGET_DIM);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SkColorSetRGB(225, 225, 225));
    canvas->drawRect(inputRect, paint);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(.1f);
    const SkFont font = toSkFont(QApplication::font(), 96, 72);

    canvas->drawString(transStr.toStdString().c_str(),
           inputRect.x() + MIN_WIDGET_DIM*0.5f,
           inputRect.y() + inputRect.height()*0.5f + FONT_HEIGHT*0.2f,
           font, paint);
}

void ValueInput::draw(QPainter *p, const int y) {
    p->setFont(QApplication::font());
    const auto transStr = getText();
    const int textWidth = QApplication::fontMetrics().width(transStr);
    const QRect inputRect(2*MIN_WIDGET_DIM, y,
                          textWidth + MIN_WIDGET_DIM, MIN_WIDGET_DIM);
    p->fillRect(inputRect, QColor(225, 225, 225));
    p->drawText(qRound(inputRect.x() + MIN_WIDGET_DIM*0.5),
                qRound(inputRect.y() + inputRect.height()*0.5 + FONT_HEIGHT*0.2),
                transStr);
}

void ValueInput::clearAndDisableInput() {
    mXYMode = MODE_XY;
    mInputEnabled = false;
    mInputText = "";
}

void ValueInput::updateInputValue() {
    if(mInputText.isEmpty()) {
        mInputEnabled = false;
    } else {
        //mFirstMouseMove = false;
        mInputEnabled = true;
        if(mInputText == "-") mInputValue = -1;
        else mInputValue = mInputText.toDouble();
    }
}
#include <QKeyEvent>
bool ValueInput::handleTransormationInputKeyEvent(const int key) {
    if(key == Qt::Key_Minus) {
        if( ((mInputText.isEmpty()) ? false : mInputText.at(0) == '-') ) {
            mInputText.remove("-");
        } else {
            mInputText.prepend("-");
        }
        updateInputValue();
    } else if(key == Qt::Key_0) {
        mInputText += "0";
        if(mInputText == "0" || mInputText == "-0") mInputText += ".";
        updateInputValue();
    } else if(key == Qt::Key_1) {
        mInputText += "1";
        updateInputValue();
    } else if(key == Qt::Key_2) {
        mInputText += "2";
        updateInputValue();
    } else if(key == Qt::Key_3) {
        mInputText += "3";
        updateInputValue();
    } else if(key == Qt::Key_4) {
        mInputText += "4";
        updateInputValue();
    } else if(key == Qt::Key_5) {
        mInputText += "5";
        updateInputValue();
    } else if(key == Qt::Key_6) {
        mInputText += "6";
        updateInputValue();
    } else if(key == Qt::Key_7) {
        mInputText += "7";
        updateInputValue();
    } else if(key == Qt::Key_8) {
        mInputText += "8";
        updateInputValue();
    } else if(key == Qt::Key_9) {
        mInputText += "9";
        updateInputValue();
    } else if(key == Qt::Key_Period || key == Qt::Key_Comma) {
        if(!mInputText.contains(".")) {
            mInputText += ".";
            updateInputValue();
        }
    } else if(key == Qt::Key_Backspace) {
        mInputText.chop(1);
        if(mInputText == "0" ||
           mInputText == "-" ||
           mInputText == "-0") mInputText = "";
        updateInputValue();
    } else {
        return false;
    }

    return true;
}
