#include "textbox.h"
#include <QInputDialog>
#include "mainwindow.h"

TextBox::TextBox(BoxesGroup *parent) : BoundingBox(parent, TYPE_TEXT)
{
    setName("text");
}

void TextBox::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        p->setOpacity(p->opacity()*mTransformAnimator.getOpacity()*0.01 );
        p->setTransform(QTransform(mCombinedTransformMatrix) );
        p->setFont(mFont);
        p->drawText(getTextRect(), mAlignment, mText);

        p->restore();
    }
}
#include <QApplication>
#include <QDesktopWidget>
QRectF TextBox::getTextRect() {
    QFontMetrics fm(mFont);
    QRectF rect = fm.boundingRect(QApplication::desktop()->geometry(),
                           mAlignment, mText);
    return rect.translated(-rect.topLeft() );
}

void TextBox::openTextEditor()
{
    bool ok;
    QString text =
            QInputDialog::getMultiLineText(mMainWindow, getName() + " text",
                                           "Text:", mText, &ok);
    if(ok) {
        mText = text;

        callUpdateSchedulers();
    }
}

bool TextBox::pointInsidePath(QPointF point)
{
    QPainterPath mapped;
    mapped.addRect(getTextRect());
    mapped = mCombinedTransformMatrix.map(mapped);
    return mapped.contains(point);
}

QRectF TextBox::getBoundingRect()
{
    QRectF rect = mCombinedTransformMatrix.mapRect(getTextRect() );
    return rect;
}

void TextBox::centerPivotPosition() {
    mTransformAnimator.setPivotWithoutChangingTransformation(getTextRect().center() );
}

void TextBox::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();

        QPainterPath mapped;
        mapped.addRect(getTextRect() );
        mapped = mCombinedTransformMatrix.map(mapped);
        QPen pen = p->pen();
        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
        p->setBrush(Qt::NoBrush);
        p->drawPath(mapped);
        p->setPen(pen);

        //drawBoundingRect(p);
        p->restore();
    }
}

void TextBox::setText(QString text)
{
    mText = text;
}

void TextBox::setCurrentText(QString text)
{
    mText = text;
}

void TextBox::setFont(QFont font)
{
    mFont = font;
}

void TextBox::setFontSize(qreal size)
{
    mFont.setPointSize(size);
}

void TextBox::setFontFamilyAndStyle(QString fontFamily, QString fontStyle)
{
    mFont.setFamily(fontFamily);
    mFont.setStyleName(fontStyle);
}
