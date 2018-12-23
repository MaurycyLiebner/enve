#include "valueinput.h"
#include <QPainter>
#include "skia/skiaincludes.h"
#include "global.h"
ValueInput::ValueInput() {

}

void ValueInput::draw(SkCanvas *canvas, const int &y) {
    if(mInputTransformationEnabled) {
        SkPaint paint;
        SkRect inputRect = SkRect::MakeXYWH(
                                2*MIN_WIDGET_HEIGHT,
                                y,
                                5*MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SkColorSetARGB(255, 225, 225, 225));
        canvas->drawRect(inputRect, paint);
        QString transStr;
        transStr = mName + ": " + mInputText + "|";

        paint.setTextSize(FONT_HEIGHT);
        SkRect bounds;
        paint.measureText(transStr.toStdString().c_str(),
                          transStr.size()*sizeof(char),
                          &bounds);
        paint.setColor(SK_ColorBLACK);
        paint.setTypeface(SkTypeface::MakeDefault());
        paint.setStyle(SkPaint::kFill_Style);

        canvas->drawString(
               transStr.toStdString().c_str(),
               inputRect.x() + paint.getTextSize(),
               inputRect.y() + inputRect.height()*0.5 + bounds.height()*0.2f,
               paint);
    }
}

void ValueInput::draw(QPainter *p, const int &y) {
    if(mInputTransformationEnabled) {
        QRectF inputRect = QRectF(
                                2*MIN_WIDGET_HEIGHT,
                                y,
                                5*MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
        p->fillRect(inputRect, QColor(225, 225, 225));
        QString transStr;
        transStr = mName + ": " + mInputText + "|";

        p->drawText(inputRect, Qt::AlignVCenter, transStr);
    }
}

void ValueInput::clearAndDisableInput() {
    mInputTransformationEnabled = false;
    mInputText = "";
}

void ValueInput::updateInputValue() {
    if(mInputText.isEmpty()) {
        mInputTransformationEnabled = false;
    } else {
        //mFirstMouseMove = false;
        mInputTransformationEnabled = true;
        mInputTransformationValue = mInputText.toDouble();
    }
}
#include <QKeyEvent>
bool ValueInput::handleTransormationInputKeyEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Minus) {
        if( ((mInputText.isEmpty()) ? false : mInputText.at(0) == '-') ) {
            mInputText.remove("-");
        } else {
            mInputText.prepend("-");
        }
        updateInputValue();
    } else if(event->key() == Qt::Key_0) {
        mInputText += "0";
        if(mInputText == "0" || mInputText == "-0") mInputText += ".";
        updateInputValue();
    } else if(event->key() == Qt::Key_1) {
        mInputText += "1";
        updateInputValue();
    } else if(event->key() == Qt::Key_2) {
        mInputText += "2";
        updateInputValue();
    } else if(event->key() == Qt::Key_3) {
        mInputText += "3";
        updateInputValue();
    } else if(event->key() == Qt::Key_4) {
        mInputText += "4";
        updateInputValue();
    } else if(event->key() == Qt::Key_5) {
        mInputText += "5";
        updateInputValue();
    } else if(event->key() == Qt::Key_6) {
        mInputText += "6";
        updateInputValue();
    } else if(event->key() == Qt::Key_7) {
        mInputText += "7";
        updateInputValue();
    } else if(event->key() == Qt::Key_8) {
        mInputText += "8";
        updateInputValue();
    } else if(event->key() == Qt::Key_9) {
        mInputText += "9";
        updateInputValue();
    } else if(event->key() == Qt::Key_Period ||
              event->key() == Qt::Key_Comma) {
        if(!mInputText.contains(".")) {
            mInputText += ".";
            updateInputValue();
        }
    } else if(event->key() == Qt::Key_Backspace) {
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
